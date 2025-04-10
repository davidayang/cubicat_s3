#include "cubicat.h"
#include "utils/helper.h"
#include <esp_wifi.h>
#include "js_binding/js_binding.h"

#define TFT_WIDTH       320
#define TFT_HEIGHT      240

#define LCD_SDA         7
#define LCD_SCL         5
#define LCD_RST         6
#define LCD_DC          4
#define LCD_TP_SDA      21
#define LCD_TP_SCL      47
#define LCD_TP_RST      17
#define LCD_TP_INT      18

#define SPKER_WS        10
#define SPKER_BCK       11
#define SPKER_DOUT      12
#define SPKER_EN        13

#define MIC_CLK         8          
#define MIC_DATA        9

FILE* openFileFlash(const char* filename, bool binary) {
    return CUBICAT.storage.openFileFlash(filename, binary);
}
FILE* openFileSD(const char* filename, bool binary) {
    return CUBICAT.storage.openFileSD(filename, binary);
}

RenderBuffer RendererDisplay::getRenderBuffer() {
    return {m_pBackBuffer, m_width, m_height};
}
// combine with renderer dirty window
void RendererDisplay::onDrawFinish(const Region& dirtyRegion) {
    m_dirtyWindow.combine({ dirtyRegion.x, dirtyRegion.y,
     int16_t(dirtyRegion.x + dirtyRegion.w), int16_t(dirtyRegion.y + dirtyRegion.h) });
}

void Cubicat::begin(bool wifiEnable, bool speakerEnable, bool micEnable, bool sdEnable)
{
    if (speakerEnable && sdEnable) {
        sdEnable = false;
        LOGW("Speaker and sd card can not be enabled simultaneously\n");
    }
    storage.init(sdEnable);
    lcd.init(TFT_WIDTH, TFT_HEIGHT, LCD_SDA, LCD_SCL, LCD_RST, LCD_DC, -1, LCD_TP_SDA, LCD_TP_SCL, LCD_TP_RST, LCD_TP_INT);
    if (speakerEnable)
        speaker.init(SPKER_BCK, SPKER_WS, SPKER_DOUT, SPKER_EN);
    if (micEnable)
        mic.init(MIC_CLK, MIC_DATA);
    if (wifiEnable)
        Wifi::init();
#if !CONFIG_REMOVE_GRAPHIC_ENGINE
    engine.createResourceManager();
    engine.createSceneManager()->createUICanvas(TFT_WIDTH, TFT_HEIGHT);
    engine.createRenderer(&lcd)->addDrawStageListener(&lcd);
    engine.createTickManager();
    engine.createScheduleManager();
#endif
}
uint32_t lastTime = 0;
void Cubicat::loop(bool present)
{
    wifi.eventLoop();
    lcd.touchLoop();
#if !CONFIG_REMOVE_GRAPHIC_ENGINE
    engine.update();
#endif
#if CONFIG_JAVASCRIPT_ENABLE
    uint32_t now = millis();
    if (lastTime == 0)
        lastTime = now;
    uint32_t elapse = now - lastTime;
    MJS_CALL("loop", 1, elapse * 0.001);
    JSShowErrorMsg();
    lastTime = now;
#endif
    if (present)
        lcd.swapBuffer();
}