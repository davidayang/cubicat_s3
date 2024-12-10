#include "tick_manager/ticke_manager.h"
#include <algorithm>

void TickManager::update(float deltaTime) {
    m_fTimeRemain += deltaTime;
    while (m_fTimeRemain >= m_fTimeInterval)
    {
        for (auto ticker : m_vTickerList) {
            ticker->onTick(m_fTimeInterval);
        }
        m_fTimeRemain -= m_fTimeInterval;
    }
}
void TickManager::addTicker(Ticker* ticker) {
    for (auto _ticker : m_vTickerList) {
        if (_ticker == ticker)
            return;
    }
    m_vTickerList.push_back(ticker);
}
void TickManager::removeTicker(Ticker* ticker) {
    m_vTickerList.erase(std::remove(m_vTickerList.begin(), m_vTickerList.end(), ticker),m_vTickerList.end());
}