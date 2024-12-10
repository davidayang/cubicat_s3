#include "renderer.h"
#include "utils/helper.h"
#include <unordered_map>
#include "definitions.h"
#include "math/constants.h"
#include "utils/logger.h"

Renderer::Renderer(uint32_t screenWidth, uint32_t screenHeight, DisplayInterface* backBuffer)
: m_pBackBufferInterface(backBuffer)
{
    assert(m_pBackBufferInterface);
    m_viewPort.x = 0;
    m_viewPort.y = 0;
    m_viewPort.w = screenWidth;
    m_viewPort.h = screenHeight;
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

#define ReadPixel(x, y, color) \
    color = backBuffer[y * m_viewPort.w + x];

void Renderer::draw(Drawable *drawable)
{
    const uint16_t *data = drawable->getDrawData();
    uint16_t* backBuffer = m_pBackBufferInterface->getBackBuffer();
    if (data && backBuffer)
    {
        auto hasMask = drawable->hasMask();
        auto maskColor = drawable->getMaskColor();
        auto pos = drawable->getPos();
        int16_t angle = drawable->getRot();
        const Vector2&  scale = drawable->getScale();
        bool hasScale = abs(scale.x - 1) > 0.001 || abs(scale.y - 1) > 0.001;
        bool hasRot = angle != 0;
        const Vector2&  size = drawable->getSize();
        const Vector2&  pivot = drawable->getPivot();
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
        if (hasRot || hasScale || drawable->getPalette()) {
            uint16_t scaler = 1 << FP_SCALE;
            int16_t scaleX = scale.x * scaler;
            int16_t scaleY = scale.y * scaler;
            for (uint16_t j = 0; j < bbox.h; ++j)
            {
                for (uint16_t i = 0; i < bbox.w; ++i)
                {
                    // _x, _y为缩放旋转后图片的像素坐标,图片中心为原点
                    int16_t _x = i + offsetx - centerX;
                    int16_t _y = j + offsety - centerY;
                    if (hasRot) {
                        int16_t _xr = ((_x * cosa - _y * sina) >> FP_SCALE);
                        int16_t _yr = ((_x * sina + _y * cosa) >> FP_SCALE);
                        _x = _xr;
                        _y = _yr;
                    }
                    if (hasScale) {
                        _x = _x * scaler / scaleX;
                        _y = _y * scaler / scaleY;
                    }
                    _x += originCenterX;
                    _y += originCenterY;
                    uint16_t color;
                    if (hasRot) {
                        if (!drawable->readPixel(_x, _y, &color))
                            continue;
                    } else {
                        color = drawable->readPixelUnsafe(_x, _y);
                    }
                    // 处理透明像素,直接跳过
                    if (hasMask && color == maskColor)
                        continue;
                    uint16_t screenX = bbox.x + i;
                    uint16_t screenY = bbox.y + j;
                    WritePixel(screenX, screenY, color)
                }
            }
        } else {
            pushImage(imgRegion.x, imgRegion.y, imgRegion.w, imgRegion.h, data, hasMask, maskColor);
        }
        drawable->setRedraw(false);
    } else {
        LOGE("draw data or back buffer is null");
    }
}
void Renderer::calculateDirtyWindow(const std::vector<DrawablePtr>& drawables) {
    m_dirtyWindow = m_lastHotRegion;
    m_dirtyWindow.combine(m_pBackBufferInterface->getForceDirtyRegion());
    std::unordered_map<uint32_t, Region> drawableRegion;
    m_hotRegion.zero();
    for (auto& drawable : drawables) {
        uint32_t id = drawable->getId();
        auto box = drawable->getBoundingBox();
        box.y = m_viewPort.h - box.y;
        drawableRegion[id] = box;
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
    m_lastDrawableRegion = drawableRegion;
    m_dirtyWindow.combine(m_hotRegion);
    VPClip(m_viewPort, m_dirtyWindow, nullptr, nullptr);
}

void Renderer::renderObjects(const std::vector<DrawablePtr>& drawables)
{
    calculateDirtyWindow(drawables);
    // only update dirty region
    if (m_dirtyWindow.w > 0 && m_dirtyWindow.h > 0) {
        // draw all drawables
        for (auto drawable : drawables)
        {
            draw(drawable);
        }
        m_pBackBufferInterface->onDrawFinish(m_dirtyWindow);
    }
    m_lastHotRegion = m_hotRegion;
}

// x, y, w, h需要是未做viewport裁切的数据，这样才能计算数据偏移
void Renderer::pushImage(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t *data, bool hasMask, uint16_t maskColor)
{
    auto backBuffer = m_pBackBufferInterface->getBackBuffer();
    Region region = {x, y, w, h};
    uint16_t offsetx, offsety;
    if (!backBuffer || !VPClip(m_dirtyWindow, region, &offsetx, &offsety)) 
        return;
    // calculate source start index
    int start_index_dest = region.y * m_viewPort.w + region.x;
    int start_index_src = offsety * w + offsetx;
    // calculate target start pointer
    auto dest_ptr = backBuffer + start_index_dest;
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
