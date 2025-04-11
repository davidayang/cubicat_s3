#ifndef _SCHEDULE_MANAGER_H_
#define _SCHEDULE_MANAGER_H_
#include "core/memory_object.h"
#include <vector>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <functional>

extern uint32_t totalId;
class Schedule : public MemoryObject
{
    friend class ScheduleManager;
private:
    template <typename CallbackT, typename... _attribs>
    Schedule(CallbackT callback, int delayMillis, _attribs... params)
    {
        m_callback = [=](void)
        {
            callback(params...);
        };
        m_exeTime = clock() + delayMillis;
        m_Id = totalId++;
        if (totalId >= UINT32_MAX)
        {
            totalId = 0;
        }
    }
    void tick(float deltaTime);
    std::function<void(void)> m_callback;
    clock_t m_exeTime;
    uint32_t m_Id;
};

class ScheduleManager
{
public:
    ScheduleManager() = default;
    void tick(float deltaTime);
    template <typename CallbackT, typename... _attribs>
    uint32_t addSchedule(CallbackT callback, int delayMillis, _attribs... params)
    {
        auto schedule = new Schedule(callback, delayMillis, params...);
        m_schedules.push_back(schedule);
        return schedule->m_Id;
    }
    void removeSchedule(uint32_t scheduleId);
    void removeAllSchedules();
private:
    std::vector<Schedule *> m_schedules;
};

#endif