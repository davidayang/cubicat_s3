#include "constants.h"
#include "definitions.h"
#include <cmath>

int16_t sinArray[360];
int16_t cosArray[360];
bool initSinCos = false;
void initSinCosValue() {
    if (!initSinCos) {
        initSinCos = true;
        uint16_t scaler = 1 << FP_SCALE;
        for (uint16_t i=0;i<360;++i) {
            sinArray[i] = (int16_t)(sinf(i * ANGLE_2_RAD) * scaler);
            cosArray[i] = (int16_t)(cosf(i * ANGLE_2_RAD) * scaler);
        }
    }
}

int16_t getSinValue(uint16_t angle) {
    initSinCosValue();
    angle %= 360;
    if (angle < 0) angle += 360;
    return sinArray[angle];
}

int16_t getCosValue(uint16_t angle) {
    initSinCosValue();
    angle %= 360;
    if (angle < 0) angle += 360;
    return cosArray[angle];
}