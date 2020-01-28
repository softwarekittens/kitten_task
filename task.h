#pragma once

#include <chrono>
#include <functional>
#include <string_view>
#include <thread>

namespace kitten {

using TaskId = uint64_t;

class Task {
public:
    Task(TaskId id, std::string_view name, std::function<void()> action);

    void run();

    friend class TaskBuilder;
    friend class Executor;
    friend class Sheduler;
private:
    TaskId m_id;
    std::string m_name;
    std::function<void()> m_action;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
    std::vector<void*> m_resources;
    std::vector<TaskId> m_depends;
};

class TaskBuilder {
public:
    TaskBuilder(Task* task) : m_task{task} {}

    template<typename Rep, typename Period>
    TaskBuilder& addDelay(const std::chrono::duration<Rep, Period>& delay) {
        m_task->m_startTime = m_task->m_startTime + delay;
        return *this;
    }

    TaskBuilder& addDependTask(TaskId dependTask);

    TaskBuilder& addSharedResource(void* sharedResource);

    TaskId getId();

private:
    Task* m_task;
};

}
