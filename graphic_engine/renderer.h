/*
* @author       Isaac
* @date         2024-08-24
* @license      MIT License
* @copyright    Copyright (c) 2024 Deer Valley
*/
#ifndef _RENDERER_H_
#define _RENDERER_H_
#include <cstdint>
#include "drawable/drawable.h"
#include "region.h"
#include <vector>
#include <unordered_map>

class DisplayInterface{
public:
    virtual void onDrawFinish(const Region& dirtyRegion) = 0;
    virtual uint16_t* getBackBuffer() = 0;
    /* Obtain the forced update dirty region for situations where the user cannot notify
    the Renderer to update the dirty region while performing custom rendering on the back buffer.*/
    virtual Region getForceDirtyRegion() {return {0, 0, 0, 0};}
};

class Renderer
{
public:
    Renderer(uint32_t viewWidth, uint32_t viewHeight, DisplayInterface* backBuffer);
    ~Renderer();
    void renderObjects(const std::vector<DrawablePtr>& objs);
private:
    void calculateDirtyWindow(const std::vector<DrawablePtr>& drawables);
    void draw(Drawable *drawable);
    void pushImage(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t *data, bool hasMask, uint16_t maskColor);
    bool                                    m_bManagedBackBuffer = true;
    Region                                  m_viewPort;
    Region                                  m_dirtyWindow;
    Region                                  m_hotRegion;
    Region                                  m_lastHotRegion;
    std::unordered_map<uint32_t, Region>    m_lastDrawableRegion;
    DisplayInterface*                    m_pBackBufferInterface = nullptr;
};
#endif