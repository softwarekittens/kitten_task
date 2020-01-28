#pragma once

#include "task.h"

#include <atomic>
#include <queue>
#include <condition_variable>
#include <thread>
#include <mutex>

namespace kitten {

class Executor;

// Поток выполнения
class Worker {
public:
    Worker(Executor* executor, uint32_t workerIndex);

    void start();

    void stop();

    void pushTasks(std::vector<std::unique_ptr<Task>> task);

private:
    void waitForTask();

private:
    Executor* m_executor;
    uint32_t m_workerIndex;
    std::atomic<bool> m_isRunning = false;
    std::mutex m_mutex;
    std::condition_variable m_waitTaskCondittion;
    std::queue<std::unique_ptr<Task>> m_tasks;
};

}
