#ifndef _DISPLAY_H_
#define _DISPLAY_H_
#include "bb_captouch.h"
#include "lcd_interface.h"
#include <algorithm>
#if !CONFIG_REMOVE_GRAPHIC_ENGINE
#include "graphic_engine/drawable/font.h"
#endif

class TouchListener {
public:
    virtual void onTouch(const TOUCHINFO& info) = 0;
    virtual void onRelease(const TOUCHINFO& info) = 0;
};

#define rgb565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

#define RED         rgb565(255,   0,   0) // 0xf800
#define GREEN       rgb565(  0, 255,   0) // 0x07e0
#define BLUE        rgb565(  0,   0, 255) // 0x001f
#define BLACK       rgb565(  0,   0,   0) // 0x0000
#define WHITE       rgb565(255, 255, 255) // 0xffff
#define GRAY        rgb565(128, 128, 128) // 0x8410
#define DARKGRAY    rgb565( 64,  64,  64) 
#define YELLOW      rgb565(255, 255,   0) // 0xFFE0
#define CYAN        rgb565(  0, 156, 209) // 0x04FA
#define PURPLE      rgb565(128,   0, 128) // 0x8010

struct DirtyWindow {
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
    void invalidate() { x1 = y1 = INT16_MAX; x2 = y2 = INT16_MIN; }
    bool valid() const { return x1 < x2 && y1 < y2; }
    void combine(const DirtyWindow& other) {
        if (!other.valid()) {
            return;
        }
        if (!valid()) {
            *this = other;
            return;
        }
        x1 = std::min(x1, other.x1);
        y1 = std::min(y1, other.y1);
        x2 = std::max(x2, other.x2);
        y2 = std::max(y2, other.y2);
    }
};

class Display {
    friend class Cubicat;
public:
    Display(const Display& ) = delete;
#ifdef CONFIG_ENABLE_TOUCH
    void setTouchListener(TouchListener* listener);
    const TOUCHINFO& getTouchInfo();
    bool isTouched();
    void touchLoop();
#endif
    uint8_t getRotation() { return m_rotation; }
    // directly push pixels to screen
    void pushPixelsToScreen(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t*);
    void swapBuffer();
    uint16_t width() { return m_width; }
    uint16_t height() { return m_height; }
    // draw primitive
    // all primitive draw directly draw to back buffer, thus coordinate is relative to view port, which means origin point (0,0)
    // is the top left corner of view port
    // [JS_BINDING_BEGIN]
    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint16_t thickness = 1);
    void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t thickness = 1);   
    void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t cornerRadius = 2);
    void drawCircle(int16_t x, int16_t y, uint16_t radius, uint16_t color, uint16_t thickness = 1);
    void fillCircle(int16_t x, int16_t y, uint16_t radius, uint16_t color);
    void fillScreen(uint16_t color);
    // [JS_BINDING_END]
    void drawImage(uint16_t x, uint16_t y, uint16_t imgWidth, uint16_t imgHeight, uint16_t* img);
    void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t thickness = 1, uint16_t cornerRadius = 2);
#if !CONFIG_REMOVE_GRAPHIC_ENGINE
    DirtyWindow drawText(uint16_t x, uint16_t y, const char* text, uint16_t color, uint8_t lineSpacing = 2, const FontData& fontData = DefaultFontData);
    DirtyWindow drawText(uint16_t x, uint16_t y, const char* text, uint16_t color, uint16_t maxWidth, uint8_t lineSpacing, const FontData& fontData = DefaultFontData);
#endif
protected:
    Display();
    ~Display();
    void init(uint16_t width, uint16_t height, int sda, int scl, int rst, int dc, int blk = -1,
        int touchSda = -1, int touchScl = -1, int touchRst = -1, int touchInt = -1);
    void allocBackBuffer();
    void _drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color, bool fill, uint16_t thickness, uint16_t cornerRadius);
    void _drawCircle(int16_t x, int16_t y, uint16_t radius, uint16_t color, bool fill, uint16_t thickness);
    bool                m_bInited = false;
#ifdef CONFIG_ENABLE_TOUCH
    BBCapTouch          m_touch;
#endif
    uint8_t             m_rotation = 0;
    uint16_t            m_width = 0;
    uint16_t            m_height = 0;
    int8_t              m_interruptGPIO = -1;
    TOUCHINFO           m_touchInfo;
    uint16_t*           m_pBackBuffer = nullptr;
    uint16_t*           m_pFrontBuffer = nullptr;
    bool                m_bDoubleBuffering = false;
    DirtyWindow         m_dirtyWindow;
    TouchListener*      m_pTouchListener = nullptr;
    bool                m_bTouched = false;
    SemaphoreHandle_t   m_bufferMutex = nullptr;
    uint16_t            m_backgroundColor = 0;
};

#endif