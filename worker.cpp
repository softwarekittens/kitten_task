#include "worker.h"
#include "executor.h"

namespace kitten {

Worker::Worker(Executor* executor, uint32_t workerIndex)
    : m_executor{executor}
    , m_workerIndex{workerIndex}
{
}

void Worker::start() {
    m_isRunning = true;
    while (m_isRunning) {
        std::unique_lock lock(m_mutex);
        std::optional<TaskId> finishedTask = std::nullopt;
        if (m_task.has_value()) {
            finishedTask = m_task->getId();
            m_task->run();
            m_task = std::nullopt;
        }
        lock.unlock();
        if (finishedTask.has_value()) {
            m_executor->onFreeWorker(this, *finishedTask);
        }
        waitForTask();
    }
}

void Worker::stop() {
    m_isRunning = false;
    m_waitTaskCondittion.notify_all();
}

void Worker::pushTask(Task task) {
    std::unique_lock lock(m_mutex);
    if (m_task.has_value()) {
        throw std::runtime_error("Fuuuu");
    }
    m_task = std::move(task);
    lock.unlock();
    m_waitTaskCondittion.notify_all();
}

void Worker::waitForTask() {
    std::unique_lock lock(m_mutex);
    if (m_isRunning && !m_task.has_value()) {
        m_waitTaskCondittion.wait(lock);
    }
}

}
