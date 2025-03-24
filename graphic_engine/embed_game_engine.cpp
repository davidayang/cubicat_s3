#include "embed_game_engine.h"
#include "component/rigidbody_component.h"
#include "utils/helper.h"

uint16_t counter = 0;
uint32_t timer0 = 0;
uint32_t timer1 = 0;
float fps = 0;

EmbedGameEngine::EmbedGameEngine()
{
}
EmbedGameEngine::~EmbedGameEngine() {
    if (m_pSceneMgr)
        delete m_pSceneMgr;
    if (m_pTickMgr)
        delete m_pTickMgr;
    if (m_pScheduleMgr)
        delete m_pScheduleMgr;
    if (m_pRenderer)
        delete m_pRenderer;
    if (m_pResourceManager)
        delete m_pResourceManager;
}
SceneManager* EmbedGameEngine::createSceneManager() {
    if (m_pSceneMgr)
        return m_pSceneMgr;
    m_pSceneMgr = new SceneManager();
    return m_pSceneMgr;
}
TickManager* EmbedGameEngine::createTickManager() {
    if (m_pTickMgr)
        return m_pTickMgr;
    m_pTickMgr = new TickManager();
    return m_pTickMgr;
}
ScheduleManager* EmbedGameEngine::createScheduleManager() {
    if (m_pScheduleMgr)
        return m_pScheduleMgr;
    m_pScheduleMgr = new ScheduleManager();
    return m_pScheduleMgr;
}
Renderer* EmbedGameEngine::createRenderer(DisplayInterface* backBuffer) {
    if (m_pRenderer)
        return m_pRenderer;
    m_pRenderer = new Renderer(backBuffer);
    return m_pRenderer;
}
ResourceManager* EmbedGameEngine::createResourceManager() {
    if (m_pResourceManager)
        return m_pResourceManager;
    m_pResourceManager = new ResourceManager();
    return m_pResourceManager;
}
void EmbedGameEngine::update() {
    if (timer0 == 0) {
        timer0 = millis();
        timer1 = millis();
    }
    uint32_t now = millis();
    uint16_t interval = ceilf(1000.0f / m_FPS);
    uint16_t elapseMS = now - timer0;
    if (elapseMS >= interval) {
        float deltaTime = elapseMS * 0.001f;
        if (m_pTickMgr)
            m_pTickMgr->update(deltaTime);
        if (m_pSceneMgr)
            m_pSceneMgr->update(deltaTime);
        if (m_pScheduleMgr)
            m_pScheduleMgr->tick(deltaTime);
        if (m_pRenderer && m_pSceneMgr)
            m_pRenderer->renderObjects(m_pSceneMgr->getDrawables());
        timer0 = now;
        counter++;
    }
    float n = 2.0f;
    if (now - timer1 >= n * 1000) {
        fps = counter / n;
        timer1 = now;
        counter = 0;
    }
}
float EmbedGameEngine::getFPS() {
    return fps;
}