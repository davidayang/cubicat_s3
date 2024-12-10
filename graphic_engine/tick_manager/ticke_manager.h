#ifndef _TICK_MANAGER_H_
#define _TICK_MANAGER_H_
#include <vector>
#include "ticker.h"
class TickManager {
public:
    TickManager() = default;
    void update(float deltaTime);
    void addTicker(Ticker* ticker);
    void removeTicker(Ticker* ticker);
private:
    std::vector<Ticker*>        m_vTickerList;
    float                       m_fTimeRemain = 0.0f;
    float                       m_fTimeInterval = 0.1f;
};
#endif