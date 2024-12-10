#include "schedule_manager.h"
#include <algorithm>

uint32_t totalId = 0;

void Schedule::tick(float deltaTime) {
    if (clock() >= m_exeTime) {
        m_callback();
    }
}

void ScheduleManager::tick(float deltaTime) {
    auto now = clock();
    for (auto schedule : m_schedules) {
        schedule->tick(deltaTime);
    }
    std::vector<Schedule*> swapList;
    for (auto schedule : m_schedules) {
        if (schedule->m_exeTime < now) {
            delete schedule;
        } else {
            swapList.push_back(schedule);
        }
    }
    m_schedules = swapList;
}

void ScheduleManager::removeSchedule(uint32_t scheduleId) {
    auto itr = std::find_if(m_schedules.begin(), m_schedules.end(), [scheduleId](Schedule* schedule) {
        return schedule->m_Id == scheduleId;
    });
    if (itr != m_schedules.end()) {
        delete *itr;
        m_schedules.erase(itr);
    }
}