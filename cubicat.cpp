#include "cubicat.h"
#include "utils/helper.h"
#include <esp_wifi.h>

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
#define SPKER_DIN       12
#define SPKER_EN        13

#define MIC_CLK         8          
#define MIC_DATA        9

void RendererDisplay::onDrawFinish(const Region& dirtyRegion) {
    m_dirtyWindow.combine({ dirtyRegion.x, dirtyRegion.y,
     int16_t(dirtyRegion.x + dirtyRegion.w), int16_t(dirtyRegion.y + dirtyRegion.h) });
}
Region RendererDisplay::getForceDirtyRegion() {
    if (!m_dirtyWindow.valid())
        return {0, 0, 0, 0};
    return {m_dirtyWindow.x1, m_dirtyWindow.y1, 
    (uint16_t)(m_dirtyWindow.x2 - m_dirtyWindow.x1), (uint16_t)(m_dirtyWindow.y2 - m_dirtyWindow.y1)};
}


void Cubicat::begin()
{
    lcd.init(TFT_WIDTH, TFT_HEIGHT, LCD_SDA, LCD_SCL, LCD_RST, LCD_DC, -1, LCD_TP_SDA, LCD_TP_SCL, LCD_TP_RST, LCD_TP_INT);
    speaker.init(SPKER_BCK, SPKER_WS, SPKER_DIN, SPKER_EN);
    mic.init(-1, MIC_CLK, MIC_DATA);
    storage.init();
    Wifi::init();
    engine.createSceneManager()->createUICanvas(TFT_WIDTH, TFT_HEIGHT);
    engine.createRenderer(TFT_WIDTH, TFT_HEIGHT, &lcd);
    engine.createTickManager();
    engine.createScheduleManager();
}
void Cubicat::loop(bool present)
{
    lcd.touchLoop();
    engine.update();
    if (present)
        lcd.swapBuffer();
}