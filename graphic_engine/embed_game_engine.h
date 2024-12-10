/*
* @author       Isaac
* @date         2024-08-20
* @license      MIT License
* @copyright    Copyright (c) 2024 Deer Valley
*/
#ifndef _EMBED_GAME_ENGINE_H_
#define _EMBED_GAME_ENGINE_H_
#include "scene_manager.h"
#include "tick_manager/ticke_manager.h"
#include "input_manager.h"
#include "component/sheet_animation_component.h"
#include "component/rigidbody_component.h"
#include "message/message_tube.h"
#include "schedule_manager.h"

class EmbedGameEngine
{
public:
    EmbedGameEngine();
    ~EmbedGameEngine();
    SceneManager* createSceneManager();
    TickManager* createTickManager();
    ScheduleManager* createScheduleManager();
    // scene graph renderer
    // @param width viewport width
    // @param height viewport height
    // @param displayInterface draw interface
    // @param displayParam custom parameter passed to display interface
    Renderer* createRenderer(uint32_t width, uint32_t height, DisplayInterface* backBuffer);
    void update();
    void setFPS(uint16_t fps) {m_FPS = fps;}
    float getFPS();
    SceneManager* getSceneManager() {return m_pSceneMgr;}
    TickManager* getTickManager() {return m_pTickMgr;}
    ScheduleManager* getScheduleManager() {return m_pScheduleMgr;}
    Renderer* getRenderer() {return m_pRenderer;}
private:
    SceneManager*                   m_pSceneMgr = nullptr;
    TickManager*                    m_pTickMgr = nullptr;
    ScheduleManager*                m_pScheduleMgr = nullptr;
    Renderer*                       m_pRenderer = nullptr;
    uint16_t                        m_FPS = 60;
};

#endif