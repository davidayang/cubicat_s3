#include "renderer.h"
#include "utils/helper.h"
#include <unordered_map>
#include "definitions.h"
#include "math/constants.h"
#include "utils/logger.h"

Renderer::Renderer(DisplayInterface* backBuffer)
: m_pBackBufferInterface(backBuffer)
{
    auto buffer = m_pBackBufferInterface->getRenderBuffer();
    m_viewPort.x = 0;
    m_viewPort.y = 0;
    m_viewPort.w = buffer.width;
    m_viewPort.h = buffer.height;
}
Renderer::~Renderer() {
}
bool VPClip(const Region& vp, Region& region, uint16_t* offsetx, uint16_t* offsety) {
    int vpEndx = vp.x + vp.w;                                    
    int vpEndy = vp.y + vp.h;      
    int regEndx = region.x + region.w;
    int regEndy = region.y + region.h;    
    if (offsetx)                          
        *offsetx = 0;       
    if (offsety)                            
        *offsety = 0;                         
    if (region.x >= vpEndx || region.y >= vpEndy || regEndx <= vp.x || regEndy <= vp.y) {
        region.zero();
        return false;                                             
    }
    if (region.x < vp.x)                                              
    {                      
        if (offsetx)                                 
            *offsetx = vp.x - region.x;                                       
        region.x = vp.x;                                              
    }                                                       
    if (region.y < vp.y)                                              
    {                      
        if (offsety)                                 
            *offsety = vp.y - region.y;                                       
        region.y = vp.y;                                              
    }                                                       
    if (regEndx > vpEndx)                                       
    {                                                       
        regEndx = vpEndx;                                       
    }                                                       
    if (regEndy > vpEndy)                                       
    {                                                       
        regEndy = vpEndy;                                       
    }                                                       
    region.w = regEndx - region.x;                                             
    region.h = regEndy - region.y;
    return true;
}

#define WritePixel(x, y, color) \
    backBuffer[y * m_viewPort.w + x] = color;

#define ReadPixel(x, y) \
    backBuffer[y * m_viewPort.w + x];

void Renderer::draw(Drawable *drawable)
{
    const void *data = drawable->getDrawData();
    uint16_t* backBuffer = m_pBackBufferInterface->getRenderBuffer().data;
    if (data && backBuffer)
    {
        auto hasMask = drawable->hasMask();
        auto hasAlpha = drawable->hasAlpha();
        auto maskColor = drawable->getMaskColor();
        auto pos = drawable->getPos();
        int16_t angle = drawable->getRot();
        const Vector2&  scale = drawable->getScale();
        bool hasScale = abs(scale.x - 1) > 0.001 || abs(scale.y - 1) > 0.001;
        bool hasRot = angle != 0;
        const Vector2&  size = drawable->getSize();
        // center point of origin picture
        uint16_t originCenterX = (uint16_t)size.x >> 1;
        uint16_t originCenterY = (uint16_t)size.y >> 1;
        //x,y 为图片左上角坐标（屏幕坐标系）
        auto bbox = drawable->getBoundingBox();
        bbox.y = m_viewPort.h - bbox.y;
        uint16_t centerX = bbox.w >> 1;
        uint16_t centerY = bbox.h >> 1;
        Region imgRegion = {bbox.x, bbox.y, bbox.w, bbox.h};
        int16_t sina = getSinValue(angle);
        int16_t cosa = getCosValue(angle);
        uint16_t offsetx = 0;
        uint16_t offsety = 0;
        if (!VPClip(m_dirtyWindow, bbox, &offsetx, &offsety))
            return;
        if (hasRot || hasScale || drawable->getPalette() || hasAlpha) {
            constexpr uint16_t SCALER = 1 << FP_SCALE;
            // int16_t scaleX = scale.x * SCALER;
            // int16_t scaleY = scale.y * SCALER;
            int16_t _offx = offsetx - centerX;
            int16_t _offy = offsety - centerY;
            uint8_t ROffset = hasAlpha? 19 : 11;
            uint8_t GOffset = hasAlpha? 13 : 5;
            uint8_t BOffset = hasAlpha? 8 : 0;
            for (uint16_t j = 0; j < bbox.h; ++j)
            {
                for (uint16_t i = 0; i < bbox.w; ++i)
                {
                    // _x, _y为缩放旋转后图片的像素坐标,图片中心为原点
#ifdef CONFIG_USE_LINEAR_FILTER
                    float _x = i + _offx;
                    float _y = j + _offy;
                    if (hasRot) {
                        float _xr = ((int)_x * cosa - (int)_y * sina) / (float)SCALER;
                        float _yr = ((int)_x * sina + (int)_y * cosa) / (float)SCALER;
                        _x = _xr;
                        _y = _yr;
                    }
#else
                    int16_t _x = i + _offx;
                    int16_t _y = j + _offy;
                    if (hasRot) {
                        int16_t _xr = (_x * cosa - _y * sina) >> FP_SCALE;
                        int16_t _yr = (_x * sina + _y * cosa) >> FP_SCALE;
                        _x = _xr;
                        _y = _yr;
                    }
#endif
                    if (hasScale) {
                        _x = _x / scale.x;
                        _y = _y / scale.y;
                    }
                    _x += originCenterX;
                    _y += originCenterY;
                    uint32_t color;
#ifdef CONFIG_USE_LINEAR_FILTER
                    // 获取四个邻近像素
                    int16_t x0 = (int16_t)_x;
                    int16_t y0 = (int16_t)_y;
                    int16_t x1 = x0 + 1;
                    int16_t y1 = y0;
                    int16_t x2 = x0;
                    int16_t y2 = y0 + 1;
                    int16_t x3 = x0 + 1;
                    int16_t y3 = y0 + 1;
                    float dx = _x - x0;
                    float dy = _y - y0;
                    float factorA = (1 - dx) * (1 - dy);
                    float factorB = dx * (1 - dy);
                    float factorC = (1 - dx) * dy;
                    float factorD = dx * dy;
                    // if (hasRot) {
                        uint32_t colorNear[4];
                        int c = 0;
                        if (drawable->readPixel(x0, y0, &colorNear[c]))
                            c++;
                        if (drawable->readPixel(x1, y1, &colorNear[c]))
                            c++;
                        if (drawable->readPixel(x2, y2, &colorNear[c]))
                            c++;
                        if (drawable->readPixel(x3, y3, &colorNear[c]))
                            c++;
                        if (c == 0) {
                            continue;
                        } else if (c == 4) {
                            uint8_t r0 = (colorNear[0] >> ROffset) & 0x1F;
                            uint8_t r1 = (colorNear[1] >> ROffset) & 0x1F;
                            uint8_t r2 = (colorNear[2] >> ROffset) & 0x1F;
                            uint8_t r3 = (colorNear[3] >> ROffset) & 0x1F;
                            uint8_t g0 = (colorNear[0] >> GOffset) & 0x3F;
                            uint8_t g1 = (colorNear[1] >> GOffset) & 0x3F;
                            uint8_t g2 = (colorNear[2] >> GOffset) & 0x3F;
                            uint8_t g3 = (colorNear[3] >> GOffset) & 0x3F;
                            uint8_t b0 = (colorNear[0] >> BOffset) & 0x1F;
                            uint8_t b1 = (colorNear[1] >> BOffset) & 0x1F;
                            uint8_t b2 = (colorNear[2] >> BOffset) & 0x1F;
                            uint8_t b3 = (colorNear[3] >> BOffset) & 0x1F;
                            uint16_t rBlend = r0 * factorA + r1 * factorB + r2 * factorC + r3 * factorD;
                            uint16_t gBlend = g0 * factorA + g1 * factorB + g2 * factorC + g3 * factorD;
                            uint16_t bBlend = b0 * factorA + b1 * factorB + b2 * factorC + b3 * factorD;
                            if (hasAlpha) {
                                uint8_t a0 = colorNear[0] & 0xff;
                                uint8_t a1 = colorNear[1] & 0xff;
                                uint8_t a2 = colorNear[2] & 0xff;
                                uint8_t a3 = colorNear[3] & 0xff;   
                                uint8_t aBlend = a0 * factorA + a1 * factorB + a2 * factorC + a3 * factorD;
                                color = (rBlend << ROffset) | (gBlend << GOffset) | (bBlend << BOffset) | aBlend;
                            } else {
                                color = (rBlend << 11) | (gBlend << 5) | bBlend;
                            }
                        } else {
                            color = colorNear[0];
                        }
                    // } else {
                    //     color = drawable->readPixelUnsafe(_x, _y);
                    // }
#else
                    if (hasRot) {
                        if (!drawable->readPixel(_x, _y, &color))
                            continue;
                    } else {
                        color = drawable->readPixelUnsafe(_x, _y);
                    }
#endif
                    // 处理透明像素,直接跳过
                    if (hasMask && color == maskColor)
                        continue;
                    uint16_t screenX = bbox.x + i;
                    uint16_t screenY = bbox.y + j;
                    // 处理alpha通道 RGBA: 5658
                    if (hasAlpha) {
                        uint16_t rgb = color >> 8;
                        uint8_t alpha = color & 0xff;
                        if (alpha == 0) {
                            continue;
                        }
                        if (alpha == 255) {
                            color = rgb;
                        } else {
                            auto bgColor = ReadPixel(screenX, screenY);
                            // mix color with bg color use alpha
                            uint8_t r = (((rgb >> 11) & 0x1F) * alpha + ((bgColor >> 11) & 0x1F) * (255 - alpha)) >> 8;
                            uint8_t g = (((rgb >> 5) & 0x3F) * alpha + ((bgColor >> 5) & 0x3F) * (255 - alpha)) >> 8;
                            uint8_t b = ((rgb & 0x1F) * alpha + (bgColor & 0x1F) * (255 - alpha)) >> 8;
                            color = (r << 11) | (g << 5) | b;
                        }
                    }
                    WritePixel(screenX, screenY, color);
                }
            }
        } else {
            pushImage(imgRegion.x, imgRegion.y, imgRegion.w, imgRegion.h, (const uint16_t *)data, hasMask, maskColor);
        }
        drawable->setRedraw(false);
    } else {
        LOGE("draw data or back buffer is null");
    }
}
void Renderer::calculateDirtyWindow(const std::vector<DrawablePtr>& drawables) {
    m_dirtyWindow = m_lastHotRegion;
    std::unordered_map<uint32_t, Region> drawableRegions;
    m_hotRegion.zero();
    for (auto& drawable : drawables) {
        uint32_t id = drawable->getId();
        auto box = drawable->getBoundingBox();
        box.y = m_viewPort.h - box.y;
        drawableRegions[id] = box;
        m_lastDrawableRegion.erase(id);
        if (drawable->needRedraw()) {
            m_hotRegion.combine(box);
        }
    }
    // drawable regions remain in m_lastDrawableRegion means they are not exist anymore, they should be marked as dirty
    for (auto& pair : m_lastDrawableRegion) {
        m_dirtyWindow.combine(pair.second);
    }
    // swap last drawable region
    m_lastDrawableRegion = drawableRegions;
    VPClip(m_viewPort, m_hotRegion, nullptr, nullptr);
    m_dirtyWindow.combine(m_hotRegion);
    VPClip(m_viewPort, m_dirtyWindow, nullptr, nullptr);
}

void Renderer::renderObjects(const std::vector<DrawablePtr>& drawables)
{
    calculateDirtyWindow(drawables);
    clear();
    for (auto it = m_drawStageListeners.rbegin(); it != m_drawStageListeners.rend(); ++it) {
        (*it)->onDrawStart(m_dirtyWindow);
    }
    // only draw dirty region
    if (m_dirtyWindow.w > 0 && m_dirtyWindow.h > 0) {
        // draw all drawables
        for (auto drawable : drawables)
        {
            draw(drawable);
        }
    }
    for (auto it = m_drawStageListeners.rbegin(); it != m_drawStageListeners.rend(); ++it) {
        (*it)->onDrawFinish(m_dirtyWindow);
    }
    m_lastHotRegion = m_hotRegion;
}

// x, y, w, h需要是未做viewport裁切的数据，这样才能计算数据偏移
void Renderer::pushImage(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t *data, bool hasMask, uint16_t maskColor)
{
    auto backBuffer = m_pBackBufferInterface->getRenderBuffer();
    Region region = {x, y, w, h};
    uint16_t offsetx, offsety;
    if (!backBuffer.data || !VPClip(m_dirtyWindow, region, &offsetx, &offsety)) 
        return;
    // calculate source start index
    int start_index_dest = region.y * m_viewPort.w + region.x;
    int start_index_src = offsety * w + offsetx;
    // calculate target start pointer
    auto dest_ptr = backBuffer.data + start_index_dest;
    auto src_ptr = data + start_index_src;
    // calculate bytes per row（2 bytes per pixel）
    int bytes_per_row = w * sizeof(uint16_t);
    // copy data to back buffer
    for (int row = 0; row < region.h; row++)
    {
        for (int col = 0; col < region.w; col++)
        {
            if (hasMask)
            {
                uint16_t color = *(src_ptr);
                if (maskColor != color)
                { // non transparent
                    *dest_ptr = *src_ptr;
                }
            }
            else
            {
                *dest_ptr = *src_ptr;
            }
            src_ptr++;
            dest_ptr++;
        }
        src_ptr += w - region.w;
        dest_ptr += m_viewPort.w - region.w;
    }
}

void Renderer::clear() {
    if (!m_bAutoClear)
        return;
    auto backBuffer = m_pBackBufferInterface->getRenderBuffer();
    auto data = backBuffer.data;
    if (!data) {
        return;
    }
    auto bgColor = m_pBackBufferInterface->getBackgroundColor();
    auto rect = m_dirtyWindow;
    rect.combine(m_lastHotRegion);
    for (int row = rect.y; row < rect.y + rect.h; row++) {
        for (int col = rect.x; col < rect.x + rect.w; col++) {
            data[row * m_viewPort.w + col] = bgColor;
        }
    }
}
void Renderer::addDrawStageListener(DrawStageListener* listener) {
    for (auto& l : m_drawStageListeners) {  
        if (l == listener) {
            return;
        }
    }
    m_drawStageListeners.push_back(listener);
}
void Renderer::removeDrawStageListener(DrawStageListener* listener) {
    for (auto it = m_drawStageListeners.begin(); it != m_drawStageListeners.end(); it++) {
        if (*it == listener) {
            m_drawStageListeners.erase(it);
            break;
        }
    }
}