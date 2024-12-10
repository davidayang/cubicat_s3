#ifndef _CUBICAT_H_
#define _CUBICAT_H_
#include "core/singleton.h"
#include "devices/devices.h"
#include "graphic_engine/embed_game_engine.h"

class RendererDisplay : public Display , public DisplayInterface{
public:
    void onDrawFinish(const Region& dirtyRegion) override;
    uint16_t* getBackBuffer() override {return m_pBackBuffer;}
    Region getForceDirtyRegion() override;
};

class Cubicat : public Singleton<Cubicat>
{
    friend class Singleton<Cubicat>;
public:
    void begin();
    void loop(bool present = true);
public:
    RendererDisplay lcd;
    Speaker         speaker;
    Microphone      mic;
    Wifi            wifi;
    UnifiedStorage  storage;
    EmbedGameEngine engine;
private:
    Cubicat() = default;
    ~Cubicat() = default;
};

#define CUBICAT Cubicat::getInstance()
#endif