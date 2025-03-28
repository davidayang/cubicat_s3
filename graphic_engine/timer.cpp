#include "timer.h"

using namespace cubicat;

Timer::Timer() {
}

void Timer::update(float deltaTime) {
    if (!m_bStop) {
        m_fElapse += deltaTime * m_direction;
        if (m_fElapse >= m_fTotalTime || m_fElapse <= 0.0f) {
            if (m_loopType == LoopType::PINGPONG) {
                m_direction *= -1;
            } else if (m_loopType == LoopType::LOOP) {
                m_fElapse = 0.0f;
            } else {
                m_bStop = true;
            }
            m_bTriggered = true;
        } else {
            m_bTriggered = false;
        }
    }
}

void Timer::reset() {
    m_fElapse = 0.0f;
}