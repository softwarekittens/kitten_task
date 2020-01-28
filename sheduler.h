#pragma once

#include "task.h"

#include <list>
#include <unordered_set>
#include <unordered_map>

namespace kitten {

class Sheduler {
public:
    Sheduler(size_t workersCount);

    void pushTask(Task task);

    void markTaskAsStarted(Task& task);

    void markTaskAsFinished(TaskId taskId);

    std::optional<Task> popTask(std::chrono::time_point<std::chrono::high_resolution_clock>* sleepedUntil = nullptr);

private:
    bool isTaskReady(Task& task) const;

    size_t m_workersCount;
    std::list<Task> m_waitingTasks;
    std::unordered_set<void*> m_lockedResources;
    std::unordered_map<TaskId, std::vector<void*>> m_runningTasks;
};

}
