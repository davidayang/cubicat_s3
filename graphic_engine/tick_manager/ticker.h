#ifndef _TICKER_H_
#define _TICKER_H_
class TickManager;
class Ticker {
    friend class TickManager;
private:
    virtual void onTick(float deltaTime) = 0;
};
#endif