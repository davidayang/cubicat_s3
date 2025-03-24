#include "common.h"
#include "../definitions.h"
#include <cmath>
namespace cubicat {
int16_t sinArray[360];
int16_t cosArray[360];
bool initSinCos = false;
void initSinCosValue() {
    if (!initSinCos) {
        initSinCos = true;
        for (uint16_t i=0;i<360;++i) {
            sinArray[i] = (int16_t)(sinf(ANGLE_2_RAD(i)) * FP_SCALE);
            cosArray[i] = (int16_t)(cosf(ANGLE_2_RAD(i)) * FP_SCALE);
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

} // namespace cubicat