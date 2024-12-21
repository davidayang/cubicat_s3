#ifndef _INPUT_MANAGER_H_
#define _INPUT_MANAGER_H_
#include <vector>
#include "core/message/message_tube.h"
#include <stdint.h>

class InputManager : protected MessageDispatcher {
public:
    void updateCursor(int16_t x,int16_t y);
    void updateOrientation(float x,float y, float z);
    void updateXYAxisAngle(float x,float y);
};
#endif