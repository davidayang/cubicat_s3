#include "cubicat.h"
#include "utils/helper.h"
#include <esp_wifi.h>
#include "js_binding/js_binding.h"

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
#ifdef CONFIG_ENABLE_LCD
#ifdef CONFIG_ENABLE_TOUCH
    lcd.init(CONFIG_LCD_WIDTH, CONFIG_LCD_HEIGHT, CONFIG_LCD_SDA, CONFIG_LCD_SCL, CONFIG_LCD_RST, CONFIG_LCD_DC, -1,
         CONFIG_TOUCH_SDA_GPIO, CONFIG_TOUCH_SCL_GPIO, CONFIG_TOUCH_RST_GPIO, CONFIG_TOUCH_INT_GPIO);
#else
    lcd.init(CONFIG_LCD_WIDTH, CONFIG_LCD_HEIGHT, CONFIG_LCD_SDA, CONFIG_LCD_SCL, CONFIG_LCD_RST, CONFIG_LCD_DC, -1,
         -1, -1, -1, -1);
#endif
#endif
    if (speakerEnable)
        speaker.init(CONFIG_SPEAKER_BCK_GPIO, CONFIG_SPEAKER_WS_GPIO, CONFIG_SPEAKER_DOUT_GPIO, CONFIG_SPEAKER_EN_GPIO);
    if (micEnable)
        mic.init(CONFIG_MIC_CLK_GPIO, CONFIG_MIC_DATA_GPIO);
    if (wifiEnable) {
        Wifi::init();
    }
#ifdef CONFIG_BT_ENABLED
    bluetooth.init();
#endif
#if !CONFIG_REMOVE_GRAPHIC_ENGINE
    engine.createResourceManager();
    engine.createSceneManager()->createUICanvas(CONFIG_LCD_WIDTH, CONFIG_LCD_HEIGHT);
    engine.createRenderer(&lcd)->addDrawStageListener(&lcd);
    engine.createTickManager();
    engine.createScheduleManager();
#endif
}
uint32_t lastTime = 0;
void Cubicat::loop(bool present)
{
    wifi.eventLoop();
#ifdef CONFIG_ENABLE_TOUCH
    lcd.touchLoop();
#endif
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