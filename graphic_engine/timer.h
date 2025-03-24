#ifndef _TIMER_H_
#define _TIMER_H_

namespace cubicat {

class Timer {
public:
    enum LoopType {
        ONCE,
        LOOP,
        PINGPONG
    };
    Timer();

    void setTime(float time) {m_fTotalTime = time;}
    void update(float deltaTime);
    float getElapse() const {return m_fElapse;}
    bool check() const {return m_fElapse > m_fTotalTime;}
    bool checkAndReset();
    void reset();
    void start() {m_bStop = false;}
    void stop() {m_bStop = true;}
    bool isStop() const {return m_bStop;}
    void setLoopType(LoopType type) {m_loopType = type; m_fElapse = 0.0f;}
    LoopType getLoopType() const {return m_loopType;}
private:
    float   m_fElapse = 0.0f;
    float   m_fTotalTime = 0.0f;
    float   m_bStop = false;
    LoopType m_loopType = LOOP;
    unsigned char  m_direction = 1;
};

}
#endif