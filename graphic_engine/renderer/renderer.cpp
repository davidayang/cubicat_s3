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
Vector4f* g_vClipPositionCache = nullptr;
uint16_t g_nClipPosCacheCount = 0;

// w and uv are already divided by w
#define CREATE_SCANLINE_EDGE(edge, p0, p1, u0, v0, u1, v1, illum0, illum1, z0, z1, w0, w1) \
{ \
    if (LIKELY(p0.y != p1.y)) { \
        float _u0 = u0; \
        float _v0 = v0; \
        float _u1 = u1; \
        float _v1 = v1; \
        float step = 1.0f / (p0.y - p1.y); \
        edge.dx = (p0.x - p1.x) * step; \
        edge.du = (_u0 - _u1) * step; \
        edge.dv = (_v0 - _v1) * step; \
        edge.dz = (z0 - z1) * step; \
        edge.dw = (w0 - w1) * step; \
        edge.dillum = (illum0 - illum1) * step; \
        if (p0.y < p1.y) { \
            edge.x = p0.x; \
            edge.yMin = p0.y; \
            edge.yMax = p1.y; \
            edge.u = _u0; \
            edge.v = _v0; \
            edge.z = z0; \
            edge.w = w0; \
            edge.illum = illum0; \
        } else { \
            edge.x = p1.x; \
            edge.yMin = p1.y; \
            edge.yMax = p0.y; \
            edge.u = _u1; \
            edge.v = _v1; \
            edge.z = z1; \
            edge.w = w1; \
            edge.illum = illum1; \
        } \
    } else { \
        edge.yMin = 0; \
        edge.yMax = 0; \
    } \
}

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

#define ReadDepth(x, y) \
    m_pZBuffer[y * m_viewport.w + x]

#define WriteDepth(x, y, z) \
    m_pZBuffer[y * m_viewport.w + x] = z;

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

#define AlphaBlend(x, y, _r, _g, _b, alpha) \
    /* if pixel has alpha channel. pixel format: RGBA8888*/ \
    if (UNLIKELY(colorMix)) { \
        _r = (_r * col_r) >> 5; \
        _g = (_g * col_g) >> 6; \
        _b = (_b * col_b) >> 5; \
    } \
    if (LIKELY(blendMode == BlendMode::Normal)) { \
        if (hasAlpha && alpha < 255) { \
            if (alpha == 0) \
                continue; \
            auto bgColor = ReadPixel(x, y); \
            _r = (_r * alpha + (bgColor >> 11) * (255 - alpha)) >> 8; \
            _g = (_g * alpha + ((bgColor >> 5) & 0x3F) * (255 - alpha)) >> 8; \
            _b = (_b * alpha + (bgColor & 0x1F) * (255 - alpha)) >> 8; \
        } \
    } else if (blendMode == BlendMode::Additive) { \
        auto bgColor = ReadPixel(x, y); \
        _r = ((_r * alpha) >> 8) + (bgColor >> 11); \
        _g = ((_g * alpha) >> 8) + ((bgColor >> 5) & 0x3F); \
        _b = ((_b * alpha) >> 8) + (bgColor & 0x1F); \
        ColorClamp(_r, _g, _b); \
    } else { /* BlendMode::Multiply */ \
        auto bgColor = ReadPixel(x, y); \
        auto bg_r = bgColor >> 11; \
        auto bg_g = (bgColor >> 5) & 0x3F; \
        auto bg_b = bgColor & 0x1F; \
        _r = ((_r * bg_r * alpha) >> 16) + ((bg_r * (255 - alpha)) >> 8); \
        _g = ((_g * bg_g * alpha) >> 16) + ((bg_g * (255 - alpha)) >> 8); \
        _b = ((_b * bg_b * alpha) >> 16) + ((bg_b * (255 - alpha)) >> 8); \
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

#define CLIP_TO_SCREEN(clip_p0, clip_p1, clip_p2, screenSize) \
    auto p0 = clipSpaceToScreenSpace(clip_p0, screenSize); \
    auto p1 = clipSpaceToScreenSpace(clip_p1, screenSize); \
    auto p2 = clipSpaceToScreenSpace(clip_p2, screenSize);

#define EDGE_FUNCTION(a, b, p, f) \
    f = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);


#define INTERPOLATE_TEXCOORD(t0, t1, t2, byc_u, byc_v, byc_w) \
    Vector2f uv; \
    uv.x = byc_u * t0.x + byc_v * t1.x + byc_w * t2.x; \
    uv.y = byc_u * t0.y + byc_v * t1.y + byc_w * t2.y;

#define INTERPOLATE_SINGLE(f0, f1, f2, byc_u, byc_v, byc_w, f) \
    f = byc_u * f0 + byc_v * f1 + byc_w * f2;


#define SCANLINE_RASTERIZE \
    if (ortho) { \
        SCAN_TRIANGLE_3D(edge0, edge1, edge2, p0, p1, p2, z0, z1, z2, 1, 1, 1, uv0, uv1, uv2, illum0, illum1, illum2) \
    } else { \
        float nearPercent = near / (far - near); \
        if (LIKELY(z0 > nearPercent && z1 > nearPercent && z2 > nearPercent)) { /*all vertex in front of camera*/  \
            SCAN_TRIANGLE_3D(edge0, edge1, edge2, p0, p1, p2, z0, z1, z2, 1/clip_p0.w, 1/clip_p1.w, 1/clip_p2.w, uv0, uv1, uv2, illum0, illum1, illum2) \
        } else if (z0 > nearPercent || z1 > nearPercent || z2 > nearPercent) { /*at least one vertex in back of camera*/  \
            SCANLINE_NEAR_PLANE_SPLIT \
        } \
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
    uint16_t texWidth = 0;
    uint16_t texHeight = 0;
    auto material = poly->getMaterial();
    auto texture = material->getTexture().get();
    auto hasAlpha = false;
    auto blendMode = material->getBlendMode();
    if (texture) {
        texWidth = texture->getTextureSize().x - 1;
        texHeight = texture->getTextureSize().y - 1;
        hasAlpha = texture->hasAlpha();
    }
    uint32_t col = material->getColor();
    uint8_t col_r = col >> 11;
    uint8_t col_g = col >> 5 & 0x3F;
    uint8_t col_b = col & 0x1F;
    bool colorMix = col != 65535;
    uint8_t r = 0, g = 0, b = 0, a = 0; uint32_t color = 0;
    auto hasRot = poly->getAngle() != 0;
    auto hasScale = poly->getScale().x != 1.0f || poly->getScale().y != 1.0f;
    auto bilinear =  material->isBilinearFilter();
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
            IntersectionPoint* p0 = &g_vIntersectionPoints[i];
            IntersectionPoint* p1 = &g_vIntersectionPoints[i + 1];
            if (p0->x > p1->x) {
                std::swap(p0, p1);
            }
            // two intersection points are out of view port
            if (p0->x < leftBorder && p1->x < leftBorder) {
                continue;
            }
            if (p0->x >= rightBorder && p1->x >= rightBorder) {
                continue;
            }
            float len = p1->x - p0->x;
            int startPos = floor(p0->x);
            float u_step = (p1->u - p0->u) / len;
            float v_step = (p1->v - p0->v) / len;
            if (p0->x >= leftBorder) {
                float subPixelHead = p0->x - startPos;
                p0->u -= u_step * subPixelHead;
                p0->v -= v_step * subPixelHead;
                len += subPixelHead;
            } else {
                float n = leftBorder - p0->x;
                startPos = leftBorder;
                p0->u += u_step * n;
                p0->v += v_step * n;
                len -= n;
            }
            float subPixelTail= p1->x - floor(p1->x);
            len -= subPixelTail;
            int16_t x = startPos - 1;
            float n = 0.0f;
            while(len > 0 && x < rightBorder - 1) {
                x++;
                n += len >= 1 ? 1.0f : len;
                len -= 1;
                // sample color only when has texture, otherwise use polygon color
                float uCoord = (p0->u + n * u_step) * texWidth;
                float vCoord = (p0->v + n * v_step) * texHeight;
#ifdef CONFIG_ENABLE_BILINEAR_FILTER
                if (hasRot || hasScale || bilinear) {
                    SamplePixelBilinear(texture, uCoord, vCoord, color);
                } else {
                    color = texture->readPixelUnsafe(uCoord, vCoord);
                }
#else
                color = texture->readPixelUnsafe(uCoord, vCoord);
#endif
                if (hasAlpha) {
                    GetRGBA32(color)
                } else {
                    GetRGB16(color)
                }
                AlphaBlend(x, y, r, g, b, a)
                CombineColor(r, g, b)
                WritePixel(x, y, color)
            }
        }
    }
}

void Renderer::draw(Drawable *drawable)
{
    uint16_t* backBuffer = m_pBackBufferInterface->getRenderBuffer().data;
    if (!backBuffer) {
        LOGE("Failed to get back buffer\n");
        return;
    }
    if (drawable->ofA<Polygon2D>()) {
        drawPolygon2DScanline(drawable->cast<Polygon2D>());
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
                if (hasAlpha) {
                    GetRGBA32(color)
                    AlphaBlend(screenX, screenY, r, g, b, a)
                    CombineColor(r, g, b)
                }
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
