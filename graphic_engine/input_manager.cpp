#include "input_manager.h"
#include "math/vector2.h"
#include "math/vector3.h"

using namespace cubicat;

void InputManager::updateCursor(int16_t x, int16_t y) {
    Vector2f pos(x, y);
    sendGlobalMessage(Msg_Cursor, &pos);
}
void InputManager::updateOrientation(float x,float y, float z) {
    Vector3f ori(x, y, z);
    sendGlobalMessage(Msg_Orientation, &ori);
}
void InputManager::updateXYAxisAngle(float x,float y) {
    Vector2f angle(x, y);
    sendGlobalMessage(Msg_XYAxisAngle, &angle);
}