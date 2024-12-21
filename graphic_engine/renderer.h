/*
* @author       Isaac
* @date         2024-08-24
* @license      MIT License
* @copyright    Copyright (c) 2024 Deer Valley
* @description  Renderer class for graphic engine, provide rendering capabilities for drawable objects.
*/
#ifndef _RENDERER_H_
#define _RENDERER_H_
#include <cstdint>
#include "drawable/drawable.h"
#include "region.h"
#include <vector>
#include <unordered_map>

struct RenderBuffer {
    uint16_t* data;
    uint16_t width;
    uint16_t height;
};

class DisplayInterface{
public:
    // offered by display device
    virtual RenderBuffer getRenderBuffer() = 0;
    virtual uint16_t getBackgroundColor() = 0;
};

class DrawStageListener{
public:
    virtual void onDrawStart(const Region& dirtyRegion) = 0;
    virtual void onDrawFinish(const Region& dirtyRegion) = 0;
};

class Renderer
{
public:
    Renderer(DisplayInterface* backBuffer);
    ~Renderer();
    void renderObjects(const std::vector<DrawablePtr>& objs);
    void addDrawStageListener(DrawStageListener* listener);
    void removeDrawStageListener(DrawStageListener* listener);
    void enableAutoClear(bool enable) { m_bAutoClear = enable; }
private:
    void calculateDirtyWindow(const std::vector<DrawablePtr>& drawables);
    void draw(Drawable *drawable);
    void pushImage(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t *data, bool hasMask, uint16_t maskColor);
    void clear();
    bool                                    m_bManagedBackBuffer = true;
    Region                                  m_viewPort;
    Region                                  m_dirtyWindow;
    Region                                  m_hotRegion;
    Region                                  m_lastHotRegion;
    std::unordered_map<uint32_t, Region>    m_lastDrawableRegion;
    DisplayInterface*                       m_pBackBufferInterface = nullptr;
    std::vector<DrawStageListener*>         m_drawStageListeners;
    bool                                    m_bAutoClear = true;
};
#endif