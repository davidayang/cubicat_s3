#ifndef _LCD_INTERFACE_H_
#define _LCD_INTERFACE_H_
#include <stdint.h>

typedef enum {DIRECTION0, DIRECTION90, DIRECTION180, DIRECTION270} DIRECTION;

#ifdef __cplusplus
extern "C" {
#endif

int initSPIBus(int sclk, int mosi, int dc, int cs, int freq);
int initLCD7789(int rst, int blk);
void lcdRotate(DIRECTION direction);
void lcdPushPixels(int x1, int y1, int x2, int y2, uint16_t* pixels);

#ifdef __cplusplus
}
#endif

#endif