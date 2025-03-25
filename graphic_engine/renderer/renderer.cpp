#include "renderer.h"
#include "utils/helper.h"
#include <unordered_map>
#include "../definitions.h"
#include "../math/common.h"
#include "utils/logger.h"
#include <string.h>
#include "../math/vector4.h"

using namespace cubicat;
IntersectionPoint* g_vIntersectionPoints = nullptr;
uint16_t g_nIntersectionPointsCount = 0;

#define GEN_INTERSECTION_POINT_2D(edge, pair, worldY) \
    if (edge->yMax != edge->yMin && worldY <= edge->yMax && worldY >= edge->yMin) { \
        IntersectionPoint& point = pair->points[pair->count++]; \
        float yStep = worldY - edge->yMin; \
        point.x = edge->x + edge->dx * yStep; \
        point.u = edge->u + edge->du * yStep; \
        point.v = edge->v + edge->dv * yStep; \
    } \
    edge++;


bool RegionClip(const Region& vp, Region& region, uint16_t* offsetx, uint16_t* offsety) {
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
    backBuffer[y * m_viewport.w + x] = color;

#define ReadPixel(x, y) \
    backBuffer[y * m_viewport.w + x];

#define SamplePixel(texture, x, y, color) \
    if (hasRot) { \
        if (!texture->readPixel(x, y, &color)) \
            continue; \
    } else { \
        color = texture->readPixelUnsafe(x, y); \
    }

#define SamplePixelBilinear(texture, x, y, color) \
    /* 获取四个邻近像素*/ \
    int16_t x0 = (int16_t)x; \
    int16_t y0 = (int16_t)y; \
    int16_t x1 = x0 + 1; \
    int16_t y1 = y0; \
    int16_t x2 = x0; \
    int16_t y2 = y0 + 1; \
    int16_t x3 = x0 + 1; \
    int16_t y3 = y0 + 1; \
    float dx = x - x0; \
    float dy = y - y0; \
    float factorD = dx * dy; \
    float factorC = dy - factorD; \
    float factorB = dx - factorD; \
    float factorA = 1 - dx - factorC; \
    uint32_t color0 = 0; \
    uint32_t color1 = 0; \
    uint32_t color2 = 0; \
    uint32_t color3 = 0; \
    int c = 0; \
    if (texture->readPixel(x0, y0, &color0)) \
        c++; \
    if (texture->readPixel(x1, y1, &color1)) \
        c++; \
    if (texture->readPixel(x2, y2, &color2)) \
        c++; \
    if (texture->readPixel(x3, y3, &color3)) \
        c++; \
    if (c == 0) { \
        continue; \
    } else if (c == 4) { \
        if (hasAlpha) { \
            uint8_t* _color0 = (uint8_t*)&color0; \
            uint8_t* _color1 = (uint8_t*)&color1; \
            uint8_t* _color2 = (uint8_t*)&color2; \
            uint8_t* _color3 = (uint8_t*)&color3; \
            uint8_t* colorOut = (uint8_t*)&color; \
            *(colorOut++) = *(_color0++) * factorA + *(_color1++) * factorB + *(_color2++) * factorC + *(_color3++) * factorD; \
            *(colorOut++) = *(_color0++) * factorA + *(_color1++) * factorB + *(_color2++) * factorC + *(_color3++) * factorD; \
            *(colorOut++) = *(_color0++) * factorA + *(_color1++) * factorB + *(_color2++) * factorC + *(_color3++) * factorD; \
            *(colorOut++) = *(_color0++) * factorA + *(_color1++) * factorB + *(_color2++) * factorC + *(_color3++) * factorD; \
        } else { \
            uint16_t rBlend = (color0 >> 11) * factorA + (color1 >> 11) * factorB + (color2 >> 11) * factorC + (color3 >> 11) * factorD; \
            uint16_t gBlend = ((color0 >> 5) & 0x3F) * factorA + ((color1 >> 5) & 0x3F) * factorB + ((color2 >> 5) & 0x3F) * factorC + ((color3 >> 5) & 0x3F) * factorD; \
            uint16_t bBlend = (color0 & 0x1F) * factorA + (color1 & 0x1F) * factorB + (color2 & 0x1F) * factorC + (color3 & 0x1F) * factorD; \
            color = (rBlend << 11) | (gBlend << 5) | bBlend; \
        } \
    } else { \
        color = color0; \
    }

#define AlphaBlend(x, y, color) \
    /* if pixel has alpha channel. pixel format: RGBA8888*/ \
    if (hasAlpha) { \
        uint8_t _r = color >> 27; \
        uint8_t _g = (color >> 18) & 0x3F; \
        uint8_t _b = (color >> 11) & 0x1F; \
        uint8_t alpha = color & 0xff; \
        if (alpha == 0) { \
            continue; \
        } else if (alpha == 255) { \
            color = (_r << 11) | (_g << 5) | _b; \
        } else { \
            auto bgColor = ReadPixel(x, y); \
            if (blendMode == BlendMode::Normal) { \
                uint8_t r = (_r * alpha + (bgColor >> 11) * (255 - alpha)) >> 8; \
                uint8_t g = (_g * alpha + ((bgColor >> 5) & 0x3F) * (255 - alpha)) >> 8; \
                uint8_t b = (_b * alpha + (bgColor & 0x1F) * (255 - alpha)) >> 8; \
                color = (r << 11) | (g << 5) | b; \
            } else if (blendMode == BlendMode::Additive) { \
                uint16_t r = ((_r * alpha) >> 8) + (bgColor >> 11); \
                if (r > 0x1F) \
                    r = 0x1F; \
                uint16_t g = ((_g * alpha) >> 8) + ((bgColor >> 5) & 0x3F); \
                if (g > 0x3F) \
                    g = 0x3F; \
                uint16_t b = ((_b * alpha) >> 8) + (bgColor & 0x1F); \
                if (b > 0x1F) \
                    b = 0x1F; \
                color = (r << 11) | (g << 5) | b; \
            } else {\
                uint8_t r = (_r * ((bgColor >> 11) & 0x1F) / 65025 * alpha + (bgColor >> 11) * (255 - alpha)) >> 8; \
                uint8_t g = (_g * ((bgColor >> 5) & 0x3F) / 65025 * alpha + ((bgColor >> 5) & 0x3F) * (255 - alpha)) >> 8; \
                uint8_t b = (_b * (bgColor & 0x1F) / 65025 * alpha + (bgColor & 0x1F) * (255 - alpha)) >> 8; \
                color = (r << 11) | (g << 5) | b; \
            } \
        } \
    } 

#define GetRGB16(color) \
    r = color >> 11; \
    g = (color >> 5) & 0x3f; \
    b = color & 0x1f;

#define GetRGBA32(color) \
    r = color >> 27; \
    g = (color >> 18) & 0x3F; \
    b = (color >> 11) & 0x1F; \
    a = color & 0xff;

#define CombineColor(r, g, b) \
    color = (r << 11) | (g << 5) | b;

#define ColorClamp(r, g, b) \
    if (r > 0x1F) r = 0x1F; \
    if (g > 0x3F) g = 0x3F; \
    if (b > 0x1F) b = 0x1F;

#define RELOCATE_BUFFER(cache, cacheType, cachedCount, newCount) \
    if (cachedCount < newCount) { \
        if (cache) \
            free(cache); \
        cachedCount = newCount; \
        cache = (cacheType*)psram_prefered_malloc(cachedCount * sizeof(cacheType)); \
    }

Renderer::Renderer(DisplayInterface* backBuffer)
: m_pBackBufferInterface(backBuffer)
{
    auto buffer = m_pBackBufferInterface->getRenderBuffer();
    m_viewport.x = 0;
    m_viewport.y = 0;
    m_viewport.w = buffer.width;
    m_viewport.h = buffer.height;
    m_pZBuffer = (uint16_t*)psram_prefered_malloc(sizeof(uint16_t) * m_viewport.w * m_viewport.h);
}
Renderer::~Renderer() {
    free(m_pZBuffer);
}

void Renderer::drawPolygon2DScanline(Polygon2D *poly) {
    uint16_t* backBuffer = m_pBackBufferInterface->getRenderBuffer().data;
    auto material = poly->getMaterial();
    auto texture = material->getTexture().get();
    auto blendMode = material->getBlendMode();
    uint16_t texWidth = texture->getTextureSize().x - 1;
    uint16_t texHeight = texture->getTextureSize().y - 1;
    bool hasAlpha = texture->hasAlpha();
    auto bilinear =  material->isBilinearFilter() || poly->getAngle() != 0;
    auto bbox = poly->getRegion();
    // change to screen coordinate
    bbox.y = m_viewport.h - bbox.y;
    RegionClip(m_dirtyWindow, bbox, nullptr, nullptr);
    int leftBorder = bbox.x;
    int rightBorder = bbox.x + bbox.w;
    int edgeCount = poly->getEdgeCount();
    auto edgeList = poly->getEdgeList();
    RELOCATE_BUFFER(g_vIntersectionPoints, IntersectionPoint, g_nIntersectionPointsCount, edgeCount);
    for (int y = bbox.y; y < bbox.y + bbox.h; y++) {
        int worldY = m_viewport.h - y;
        int aeCount = 0;
        for (int i=0;i<edgeCount;i++) {
            auto& e = edgeList[i];
            if (worldY <= e.yMax && worldY >= e.yMin) {
                IntersectionPoint* p = &g_vIntersectionPoints[aeCount++];
                p->x = e.x + e.dx * (worldY - e.yMin);
                p->u = e.u + e.du * (worldY - e.yMin);
                p->v = e.v + e.dv * (worldY - e.yMin);
            }
        }
        for (int i = 0; i < aeCount; i+=2) {
            IntersectionPoint& p0 = g_vIntersectionPoints[i];
            IntersectionPoint& p1 = g_vIntersectionPoints[i + 1];
            if (p0.x > p1.x) {
                std::swap(p0, p1);
            }
            // two intersection points are close than 1 pixel
            float len = p1.x - p0.x;
            if (len < 1.0f) {
                len = 1.0f;
            }
            int startPos = p0.x;
            int endPos = p1.x;
            float u_step = (p1.u - p0.u) / len;
            float v_step = (p1.v - p0.v) / len;
            float u_offset = u_step * (startPos - p0.x);
            float v_offset = v_step * (startPos - p0.x);
            // handle out of bounding box
            if (p0.x < bbox.x) {
                startPos = bbox.x;
                u_offset = u_step * (bbox.x - p0.x);
                v_offset = v_step * (bbox.x - p0.x);
            }
            if (p1.x >= bbox.x + bbox.w) {
                endPos = bbox.x + bbox.w;
            }
            for (int x = startPos; x < endPos; x++) {
                // calc u, v and epsilon for percision correction
                const float epsilon = 0.0001f;
                float u = p0.u + u_offset + epsilon;
                float v = p0.v + v_offset + epsilon;
                u_offset += u_step;
                v_offset += v_step;
                float uCoord = u * texWidth;
                float vCoord = v * texHeight;
                uint8_t r = 0, g = 0, b = 0, a = 255; uint32_t color = 0;
#ifdef CONFIG_ENABLE_BILINEAR_FILTER
                if (bilinear) {
                    SamplePixelBilinear(texture, uCoord, vCoord, color);
                } else {
                    color = texture->readPixelUnsafe(uCoord, vCoord);
                }
#else
                color = texture->readPixelUnsafe(uCoord, vCoord);
#endif
                AlphaBlend(x, y, color);
                WritePixel(x, y, color);
            }
        }
    }
}

void Renderer::draw(Drawable *drawable)
{
    uint16_t* backBuffer = m_pBackBufferInterface->getRenderBuffer().data;
    if (drawable->ofA<Polygon2D>()) {
        drawPolygon2DScanline(static_cast<Polygon2D*>(drawable));
        drawable->onFinishDraw();
        return;
    }
    auto material = drawable->getMaterial();
    auto texture = material->getTexture();
    const void *data = texture->getTextureData();
    auto hasMask = material->hasMask();
    auto maskColor = material->getMaskColor();
    auto hasAlpha = texture->hasAlpha();
    auto blendMode = material->getBlendMode();
    int16_t angle = drawable->getAngle();
    const Vector2f&  scale = drawable->getScale();
    bool hasScale = abs(scale.x - 1) > 0.001 || abs(scale.y - 1) > 0.001;
    bool hasRot = angle % 360 != 0;
    const Vector2us& size = texture->getTextureSize();
    // center point of origin picture
    uint16_t originCenterX = (uint16_t)size.x >> 1;
    uint16_t originCenterY = (uint16_t)size.y >> 1;
    //x,y 为图片左上角坐标（屏幕坐标系）
    auto region = drawable->getRegion();
    region.y = m_viewport.h - region.y;
    uint16_t centerX = region.w >> 1;
    uint16_t centerY = region.h >> 1;
    Region imgRegion = region;
    int16_t sina = getSinValue(angle);
    int16_t cosa = getCosValue(angle);
    uint16_t offsetx = 0;
    uint16_t offsety = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
    uint32_t color = 0;
    uint32_t col = material->getColor();
    uint8_t col_r = col >> 11;
    uint8_t col_g = col >> 5 & 0x3F;
    uint8_t col_b = col & 0x1F;
    bool colorMix = col != 65535;
    // 只绘制脏区域内的像素
    if (!RegionClip(m_dirtyWindow, region, &offsetx, &offsety))
        return;
    if (hasRot || hasScale || texture->getPalette() || hasAlpha) {
        int16_t _offx = offsetx - centerX;
        int16_t _offy = offsety - centerY;
        for (uint16_t j = 0; j < region.h; ++j)
        {
            for (uint16_t i = 0; i < region.w; ++i)
            {
                // _x, _y为缩放旋转后图片的像素坐标,图片中心为原点
#ifdef CONFIG_ENABLE_BILINEAR_FILTER
                float _x = i + _offx;
                float _y = j + _offy;
                if (hasRot) {
                    float _xr = ((int)_x * cosa - (int)_y * sina) / (float)FP_SCALE;
                    float _yr = ((int)_x * sina + (int)_y * cosa) / (float)FP_SCALE;
                    _x = _xr;
                    _y = _yr;
                }
#else
                int16_t _x = i + _offx;
                int16_t _y = j + _offy;
                if (hasRot) {
                    int16_t _xr = (_x * cosa - _y * sina) >> FP_SCALE_SHIFT;
                    int16_t _yr = (_x * sina + _y * cosa) >> FP_SCALE_SHIFT;
                    _x = _xr;
                    _y = _yr;
                }
#endif
                if (hasScale) {
                    _x /= scale.x;
                    _y /= scale.y;
                }
                // 转换到图片坐标
                _x += originCenterX;
                _y += originCenterY;
#ifdef CONFIG_ENABLE_BILINEAR_FILTER
                if (hasRot || hasScale) {
                    SamplePixelBilinear(texture, _x, _y, color);
                } else {
                    SamplePixel(texture, _x, _y, color);
                }
#else
                SamplePixel(texture, _x, _y, color);
#endif
                uint16_t screenX = region.x + i;
                uint16_t screenY = region.y + j;
                AlphaBlend(screenX, screenY, color);
                WritePixel(screenX, screenY, color);
            }
        }
    } else {
        pushImage(imgRegion.x, imgRegion.y, imgRegion.w, imgRegion.h, (const uint16_t *)data, hasMask, maskColor);
    }
    drawable->onFinishDraw();
}
void Renderer::calculateDirtyWindow(const std::vector<DrawablePtr>& drawables) {
    m_dirtyWindow = m_lastHotRegion;
    std::unordered_map<uint32_t, Region> drawableRegions;
    m_hotRegion.zero();
    for (auto& drawable : drawables) {
        uint32_t id = drawable->getId();
        auto region = drawable->getRegion();
        region.y = m_viewport.h - region.y;
        drawableRegions[id] = region;
        m_lastDrawableRegion.erase(id);
        if (drawable->getDirtyAndClear())
            m_hotRegion.combine(region);
    }
    // If drawable regions remain in m_lastDrawableRegion, it means they no longer exist and should be marked as dirty.
    for (auto& pair : m_lastDrawableRegion) {
        m_dirtyWindow.combine(pair.second);
    }
    // swap last drawable region
    m_lastDrawableRegion = drawableRegions;
    RegionClip(m_viewport, m_hotRegion, nullptr, nullptr);
    m_dirtyWindow.combine(m_hotRegion);
    RegionClip(m_viewport, m_dirtyWindow, nullptr, nullptr);
}

void Renderer::renderObjects(const std::vector<DrawablePtr>& drawables)
{
    calculateDirtyWindow(drawables);
    clear();
    for (auto it = m_drawStageListeners.rbegin(); it != m_drawStageListeners.rend(); ++it) {
        (*it)->onDrawStart(m_dirtyWindow);
    }
    BENCHMARK_CLASS_RESET
    // only draw dirty region
    if (m_dirtyWindow.valid()) {
        for (auto& drawable : drawables)
        {
            draw(drawable);
        }
    }
    BENCHMARK_CLASS_REPORT
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
    if (!backBuffer.data || !RegionClip(m_dirtyWindow, region, &offsetx, &offsety)) 
        return;
    // calculate source start index
    int start_index_dest = region.y * m_viewport.w + region.x;
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
        dest_ptr += m_viewport.w - region.w;
    }
}

void Renderer::clear() {
    if (!m_bAutoClear)
        return;
    auto backBuffer = m_pBackBufferInterface->getRenderBuffer();
    if (!backBuffer.data) {
        return;
    }
    auto bgColor = m_pBackBufferInterface->getBackgroundColor();
    RegionClip(m_viewport, m_dirtyWindow, nullptr, nullptr);
    const uint16_t maxDepth = 65535; 
    if (m_dirtyWindow == m_viewport) {
        uint32_t count = m_viewport.w * m_viewport.h;
        std::fill(backBuffer.data, backBuffer.data + count, bgColor);
        std::fill(m_pZBuffer, m_pZBuffer + count, maxDepth);
    } else {
        for (auto row = m_dirtyWindow.y; row < m_dirtyWindow.y + m_dirtyWindow.h; row++) {
            uint32_t offset = row * m_viewport.w + m_dirtyWindow.x;
            uint16_t* addrBackBuffer = backBuffer.data + offset;
            uint16_t* addrZBuffer = m_pZBuffer + offset;
            std::fill(addrBackBuffer, addrBackBuffer + m_dirtyWindow.w, bgColor);
            std::fill(addrZBuffer, addrZBuffer + m_dirtyWindow.w, maxDepth);
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

// // accelerte sampling by xtensa SIMD
// inline void textureSampling32NormalFast(int16_t* uCoords, int16_t* vCoords, const uint32_t* texture, uint16_t* backBuffer, uint16_t texWidth,
//     int xStart, int xEnd, int y, int viewWidth) {
//     int len = xEnd - xStart;
//     if (len <= 0)
//         return;
//     int16_t* uAddr = uCoords + xStart;
//     int16_t* vAddr = vCoords + xStart;
//     uint16_t* backBufferAddr = backBuffer + viewWidth * y;
//     // a8 u coord, new tex address
//     // a9 v coord, offset
//     // a10 color, bg color
//     // a11 r 
//     // a12 g
//     // a13 b
//     // a14 a, temp
//     // a15 temp
//     __asm__ volatile(
//         "loopnez    %[l], .loop_end     \n"
//             "l16si  a8, %[u], 0         \n" // load uCoords value
//             "l16si  a9, %[v], 0         \n" // load vCoords value
//             "addi.n %[u], %[u], 2       \n" // uAddr++
//             "addi.n %[v], %[v], 2       \n" // vAddr++
//             "mull   a9, a9, %[tw]       \n" //  v * texWidth
//             "add    a9, a9, a8          \n" // texure ptr address offset
//             "slli   a9, a9, 1           \n" // offset * 2
//             "add    a8, %[t], a9        \n" // save new texure address to a8
//             "l32i   a10, a8, 0          \n" // load texture color
//             "srli   a11, a10, 27        \n" // color(32 bit) >> 27 : r
//             "srli   a12, a10, 18        \n" // color(32 bit) >> 18 : g
//             "movi.n a14, 0x3f           \n"
//             "and    a12, a12, a14       \n" // to 6 bit
//             "srli   a13, a10, 11        \n" // color(32 bit) >> 11 : b
//             "movi.n a14, 0x1f           \n"
//             "and    a13, a14, a14       \n" // to 5 bit
//             "movi.n a14, 0xff           \n"
//             "and    a14, a10, a14       \n" // get alpha
//             "l32i   a10, %[b], 0        \n" // load bg color for blending
//             "or     a10, a11, a12       \n"
//             "or     a10, a10, a13     \n"
//             "s16i   a10, %[b], 0                  \n"
//             "addi   %[b], %[b], 2       \n" // back buffer address++
//         ".loop_end:                     \n"
//         :
//         : [u] "a" (uAddr), [v] "a" (vAddr), [t] "a" (texture), [b] "a" (backBufferAddr), [tw] "a" (texWidth), [l] "a" (len)
//         : "memory"
//     );
// }
