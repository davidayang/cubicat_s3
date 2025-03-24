#include "id_object.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

unsigned int IDObject::m_nIdCounter = 0;

IDObject::IDObject() {
    m_nId = ++m_nIdCounter;
}