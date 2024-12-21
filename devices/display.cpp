#include "display.h"
#include "esp_psram.h"
#include "esp_heap_caps.h"
#include "utils/logger.h"
#include "utils/helper.h"
#include <cmath>
#include <string.h>

QueueHandle_t swapQueue;
volatile bool g_bPresented = true;

struct SwapBufferDesc {
    uint16_t* buffer;
    DirtyWindow dirtyWindow;
    uint16_t viewWidth;
    uint16_t viewHeight;
    TFT_t* tft;
};

class BufferLock {
public:
    BufferLock() = delete;
    BufferLock(SemaphoreHandle_t lock) : m_lock(lock) {
        xSemaphoreTake(m_lock, portMAX_DELAY);
    }
    ~BufferLock() {
        xSemaphoreGive(m_lock);
    }
private:
    SemaphoreHandle_t m_lock;
};
#define LOCK \
    BufferLock lock(m_bufferMutex);

void swapBufferTask(void* param) {
    while (1)
    {
        SwapBufferDesc desc;
        if (xQueueReceive(swapQueue, &desc, portMAX_DELAY) == pdTRUE) {
            lcdPushPixels(desc.tft, desc.dirtyWindow.x1, desc.dirtyWindow.y1, 
            desc.dirtyWindow.x2, desc.dirtyWindow.y2,desc.buffer);
            g_bPresented = true;
        } else {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}

Display::Display()
#ifdef CONFIG_DOUBLE_BUFFERING
 : m_bDoubleBuffering(true)
#endif
{
    m_touchInfo.count = 0;
    m_dirtyWindow.invalidate();
}
Display::~Display()
{
    if (m_pBackBuffer) {
        free(m_pBackBuffer);
        m_pBackBuffer = nullptr;
    }
    if (m_pFrontBuffer) {
        free(m_pFrontBuffer);
        m_pFrontBuffer = nullptr;
    }
    if (m_bufferMutex) {
        vSemaphoreDelete(m_bufferMutex);
        m_bufferMutex = nullptr;
    }
}

void Display::init(uint16_t width, uint16_t height, int sda, int scl, int rst, int dc, int blk, int touchSda,
 int touchScl, int touchRst, int touchInt)
{
    if (m_bInited)
        return;
    m_width = width;
    m_height = height;
    allocBackBuffer();
    m_interruptGPIO = touchInt;
    m_rotation = width>height ? 1 : 0;
    spi_master_init(&m_dev, sda, scl, -1, dc, rst, blk);
	lcdInit(&m_dev, width, height, 0, 0);
    if (touchSda > 0 && touchScl > 0 && touchRst > 0) {
        m_touch.init(touchSda, touchScl, touchRst, touchInt);
    }
    m_bInited = true;
    // cubicat uses a protrait screen, if we want a landscape screen, rotate the screen 90 degrees
    if (width > height)
        lcdRotate(&m_dev, DIRECTION90);
#ifdef CONFIG_DOUBLE_BUFFERING
    if (m_bDoubleBuffering) {
        swapQueue = xQueueCreate(1, sizeof(SwapBufferDesc));
        xTaskCreatePinnedToCore(swapBufferTask, "swap buffer", 1024*4, this, 1, NULL, getSubCoreId());
    }
#endif
    // first swap to clear the screen
    swapBuffer();
}
void Display::setTouchListener(TouchListener* callback) {
    m_pTouchListener = callback;
}
const TOUCHINFO& Display::getTouchInfo() {
    if (isTouched()) {
        m_touch.getSamples(&m_touchInfo);
        for (int i = 0; i < m_touchInfo.count; i++) {
            if (m_rotation == 1) {
                uint16_t temp = m_touchInfo.x[i];
                m_touchInfo.x[i] = m_touchInfo.y[i];
                m_touchInfo.y[i] = m_height - temp;
            } else if (m_rotation == 2) {
                m_touchInfo.x[i] = m_width - m_touchInfo.x[i];
                m_touchInfo.y[i] = m_height - m_touchInfo.y[i];
            } else if (m_rotation == 3) {
                uint16_t temp = m_touchInfo.x[i];
                m_touchInfo.x[i] = m_width - m_touchInfo.y[i];
                m_touchInfo.y[i] = temp;
            }
        }
    }
    return m_touchInfo;
}
bool Display::isTouched() {
    return gpio_get_level((gpio_num_t)m_interruptGPIO) == 0;
}
void Display::touchLoop() {
    if (m_pTouchListener && isTouched()) {
        auto& info = getTouchInfo();
        m_pTouchListener->onTouch(info);
    }
}

void Display::allocBackBuffer() {
    if (m_pBackBuffer)
        return;
    uint32_t size = m_width * m_height * sizeof(uint16_t);
    if (esp_psram_init()) {
        m_pBackBuffer = (uint16_t*)heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        if (!m_pBackBuffer)
            m_bDoubleBuffering = false;
    } else {
        m_bDoubleBuffering = false;
    }
    if (!m_pBackBuffer)
        m_pBackBuffer = (uint16_t *)malloc(size);
    assert(m_pBackBuffer);
    memset(m_pBackBuffer,0, size);
#ifdef CONFIG_DOUBLE_BUFFERING
    if (m_bDoubleBuffering) {
        m_pFrontBuffer = (uint16_t*)heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        if (m_pFrontBuffer)
            memset(m_pFrontBuffer,0, size);
        else
            m_bDoubleBuffering = false;
    }
#endif
    m_bufferMutex = xSemaphoreCreateMutex();
}   
void Display::pushPixelsToScreen(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t* pixels) {
    lcdPushPixels(&m_dev, x1, y1, x2, y2, pixels);
}
void Display::swapBuffer() {
    if (!m_dirtyWindow.valid())
        return;
    int16_t x1 = 0;
    int16_t y1 = m_dirtyWindow.y1;
    int16_t x2 = m_width - 1;
    int16_t y2 = m_dirtyWindow.y2;
    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    if (x2 >= m_width)
        x2 = m_width - 1;
    if (y2 >= m_height)
        y2 = m_height - 1;
    uint16_t rectWidth = x2 - x1 + 1;
    uint16_t rectHeight = y2 - y1 + 1;
#ifdef CONFIG_DOUBLE_BUFFERING
    if (m_bDoubleBuffering) {
        if (g_bPresented) {
            memcpy(m_pFrontBuffer + y1 * m_width, m_pBackBuffer + y1 * m_width, rectWidth * rectHeight * sizeof(uint16_t));
            SwapBufferDesc desc;
            desc.buffer = m_pFrontBuffer + y1 * m_width;
            desc.dirtyWindow = { x1, y1, x2, y2};
            desc.tft = &m_dev;
            desc.viewHeight = m_height;
            desc.viewWidth = m_width;
            if (xQueueSend(swapQueue, &desc, portMAX_DELAY) == pdTRUE) {
                g_bPresented = false;
            }
        }
    } else {
#endif
        LOCK
        lcdPushPixels(&m_dev, x1, y1, x2, y2, m_pBackBuffer + y1 * m_width);
#ifdef CONFIG_DOUBLE_BUFFERING
    }
#endif
    m_dirtyWindow.invalidate();
}

#define DrawPixel(x, y, color) \
    m_pBackBuffer[y * m_width + x] = color;

void Display::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint16_t thickness) {
    allocBackBuffer();
    int16_t minX = std::min(x1, x2);
    int16_t maxX = std::max(x1, x2);
    int16_t minY = std::min(y1, y2);
    int16_t maxY = std::max(y1, y2);
    // early out
    if (x1 == x2 && y1 == y2) {
        return;
    }
    if ( minX > m_width || maxX < 0 || minY > m_height || maxY < 0) {
        return;
    }
    LOCK
    m_dirtyWindow.combine({ minX, minY, maxX, maxY });
    int i;
    int dx, dy;
    int sx, sy;
    int E;

    // distance between two points
    dx = maxX - minX;
    dy = maxY - minY;
    // direction of two point
    sx = (x2 > x1) ? 1 : -1;
    sy = (y2 > y1) ? 1 : -1;
    // inclination < 1 
    if (dx > dy) {
        E = -dx;
        for (i = 0; i <= dx; i++) {
            if (x1 >= 0 && x1 < m_width && y1 >= 0 && y1 < m_height)
                DrawPixel(x1, y1, color);
            x1 += sx;
            E += 2 * dy;
            if (E >= 0) {
                y1 += sy;
                E -= 2 * dx;
            }
        }
    } else { //inclination >= 1
        E = -dy;
        for (i = 0; i <= dy; i++) {
            if (x1 >= 0 && x1 < m_width && y1 >= 0 && y1 < m_height)
                DrawPixel(x1, y1, color);
            y1 += sy;
            E += 2 * dx;
            if (E >= 0) {
                x1 += sx;
                E -= 2 * dy;
            }
        }
    }
}
void Display::_drawRect(int16_t xs, int16_t ys, uint16_t w, uint16_t h, uint16_t color, bool fill, uint16_t thickness, uint16_t cornerRadius) {
    allocBackBuffer();
    // early out
    if (xs + w <= 0 || ys + h <= 0 || xs >= m_width || ys >= m_height)
        return;
    LOCK
    m_dirtyWindow.combine({ xs, ys, int16_t(xs + w - 1), int16_t(ys + h - 1) });
    if (fill && cornerRadius == 0) {
        if (xs < 0) {
            xs = 0;
        }
        if (ys < 0) {
            ys = 0;
        }
        if (xs + w > m_width) {
            w = m_width - xs;
        }
        if (ys + h > m_height) {
            h = m_height - ys;
        }
        for (int y = ys; y < ys + h; ++y) {
            for (int x = xs; x < xs + w; ++x) {
                m_pBackBuffer[y * m_width + x] = color;
            }
        }
        return;
    }
    if (thickness == 0) {
        thickness = 1;
    }
    // todo 暂时不考虑lineWidth 大于 cornerRadius的情况
    if (cornerRadius > 0 && thickness > 2*cornerRadius) {
        thickness = 2*cornerRadius;
    }
    auto minValue = std::min(w, h);
    if (cornerRadius > (minValue >> 1)) {
        cornerRadius = (minValue >> 1);
    }
    // rectangle area corner
    int16_t rx1 = xs + cornerRadius;
    int16_t ry1 = ys + cornerRadius;
    int16_t rx2 = rx1 + w - 2 * cornerRadius;
    int16_t ry2 = ry1 + h - 2 * cornerRadius;
    int16_t centerL = xs + thickness;
    int16_t centerR = xs + w - thickness;
    int16_t centerT = ys + thickness;
    int16_t centerB = ys + h - thickness;
    for (int16_t y= ys; y < ys+h; ++y) {
        for (int16_t x = xs; x < xs+w; ++x) {
            // view port clip
            if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
                continue;
            }
            // draw rectangle sections
            if ((x >= rx1 && x <= rx2) || (y >= ry1 && y <= ry2)) {
                if (fill || x < centerL || x >= centerR || y < centerT || y >= centerB) {
                    DrawPixel(x, y, color);
                }
            } else { // drw 4 rectangle corner sections
                bool leftTop = x <= rx1 && y <= ry1;
                bool rightTop = x > rx2 && y <= ry1;
                bool leftBottom = x <= rx1 && y > ry2;
                bool rightBottom = x > rx2 && y > ry2;
                int16_t _x = 0;
                int16_t _y = 0;
                if (leftTop) {
                    _x = x - rx1;
                    _y = y - ry1;
                } else if (rightTop) {
                    _x = x - rx2;
                    _y = y - ry1;
                } else if (leftBottom) {
                    _x = x - rx1;
                    _y = y - ry2;
                } else if (rightBottom) {
                    _x = x - rx2;
                    _y = y - ry2;
                }
                if (leftTop || rightTop || leftBottom || rightBottom) {
                    auto distSqr = _x * _x + _y * _y;
                    if (distSqr <= cornerRadius * cornerRadius) {
                        if (fill || distSqr >= (cornerRadius - thickness) * (cornerRadius - thickness))
                            DrawPixel(xs+x, ys+y, color);
                    }
                }
            }
        }
    }
}
void Display::drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t thickness) {
    _drawRect(x, y, w, h, color, false, thickness, 0);
}
void Display::drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t thickness, uint16_t cornerRadius) {
    _drawRect(x, y, w, h, color, false, thickness, cornerRadius);
}
void Display::fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) {
    _drawRect(x, y, w, h, color, true, 0, 0);
}
void Display::fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t cornerRadius) {
    _drawRect(x, y, w, h, color, true, 0, cornerRadius);
}
void Display::_drawCircle(int16_t x, int16_t y, uint16_t radius, uint16_t color, bool fill, uint16_t thickness) {
    allocBackBuffer();
    // early out
    if (radius == 0 || x + radius < 0 || x - radius >= m_width || y + radius < 0 || y - radius >= m_height) {
        return;
    }
    LOCK
    m_dirtyWindow.combine({ int16_t(x - radius), int16_t(y - radius), int16_t(x + radius), int16_t(y + radius) });
    for (int16_t y1 = y - radius; y1 <= y + radius; y1++) {
        for (int16_t x1 = x - radius; x1 <= x + radius; x1++) {
            int16_t dx = x1 - x;
            int16_t dy = y1 - y;
            uint16_t distSqr = dx * dx + dy * dy;
            if (distSqr <= radius * radius) {
                if (fill || distSqr >= (radius - thickness) * (radius - thickness)) {
                    DrawPixel(x1, y1, color);
                }
            }
        }
    }
}
void Display::drawCircle(int16_t x, int16_t y, uint16_t r, uint16_t color, uint16_t thickness) {
    _drawCircle(x, y, r, color, false, thickness);
}
void Display::fillCircle(int16_t x, int16_t y, uint16_t r, uint16_t color) {
    _drawCircle(x, y, r, color, true, 0);
}
void Display::fillScreen(uint16_t color) {
    fillRect(0, 0, m_width, m_height, color);
    m_backgroundColor = color;
}
void Display::drawImage(uint16_t x, uint16_t y, uint16_t imgWidth, uint16_t imgHeight, uint16_t* img) {
    if (x >= m_width || y >= m_height)
        return;
    LOCK
    uint16_t width = imgWidth;
    uint16_t height = imgHeight;
    m_dirtyWindow.combine({ (int16_t)x, (int16_t)y, (int16_t)(x + width), (int16_t)(y + height) });
    if (x + width > m_width)
        width = m_width - x;
    if (y + height > m_height)
        height = m_height - y;
    for (int y = 0; y < height; y++) {
        memcpy(m_pBackBuffer + x + y * m_width, img + y * imgWidth, width * sizeof(uint16_t));
    }
}
#if !CONFIG_REMOVE_GRAPHIC_ENGINE
void Display::drawText(uint16_t xs, uint16_t ys, const char* text, uint16_t color, uint8_t lineSpacing, const FontData& fontData) {
    int16_t cursorPosX = xs;
    int16_t cursorPosY = ys;
    auto characters = splitUTF8(text);
    LOCK
    // single glyph data len = 4 byte bbox + glyph width * glyph height / 8 (1 bit per pixel , 8 pixels for 1 byte) 
    const uint16_t SingleGlyphDataLen = 4 + ceil(fontData.fontSize * fontData.fontSize / 8.0f);
    int16_t maxCursorY = 0;
    for (auto& character : characters) {
        uint8_t glyphWidth = fontData.fontSize;
        int index = getCharIndex((const char*)fontData.charSet, character.c_str());
        if (index >= 0) {
            auto ptr = fontData.glyphData + index * SingleGlyphDataLen;
            // 字模数据前4字节为字模bbox数据
            uint8_t data[4] = { 0 };
            memcpy(data, (uint8_t*)ptr, 4);
            uint8_t x_offset = data[0];
            uint8_t y_offset = data[1];
            glyphWidth = data[2] - data[0];
            uint8_t glyphHeight = data[3] - data[1];
            ptr += 4; // 跳过前面4字节bbox数据，开始字模数据
            for (uint8_t y = 0; y < glyphHeight; y++) {
                for (uint8_t x = 0; x < glyphWidth; x++) {
                    uint16_t vposx = cursorPosX + x + x_offset;
                    uint16_t vposy = cursorPosY + y + y_offset;
                    // 检测是否超出了view port范围
                    if (vposx > m_width - 1 || vposy > m_height - 1) {
                        continue;
                    }
                    // 字模中的数据偏移量
                    auto offset = y * glyphWidth + x;
                    uint8_t v = ptr[offset / 8] & (1 << (7 - offset % 8));
                    if (v) {
                        DrawPixel(vposx, vposy, color);
                    }
                }
            }
            int curEndY = cursorPosY + y_offset + glyphHeight; 
            if (curEndY > maxCursorY)
                maxCursorY = curEndY;
        }
        cursorPosX += glyphWidth;
        if (character == "\n") {
            cursorPosX = xs;
            cursorPosY += fontData.fontSize + lineSpacing;
        }
    }
    m_dirtyWindow.combine({ (int16_t)xs, (int16_t)ys, cursorPosX, maxCursorY});
}
#endif