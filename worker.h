#pragma once

#include "task.h"

#include <atomic>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <optional>

namespace kitten {

class Executor;

// Поток выполнения
class Worker {
public:
    Worker(Executor* executor, uint32_t workerIndex);

    void start();

    void stop();

    void pushTask(Task task);

private:
    void waitForTask();

private:
    Executor* m_executor;
    uint32_t m_workerIndex;
    std::atomic<bool> m_isRunning = false;
    std::mutex m_mutex;
    std::condition_variable m_waitTaskCondittion;
    std::optional<Task> m_task;
};

}
