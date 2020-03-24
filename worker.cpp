#include "worker.h"
#include "executor.h"

namespace kitten {

Worker::Worker(Executor* executor, uint32_t workerIndex)
    : m_executor(executor)
    , m_workerIndex(workerIndex)
{
}

void Worker::start(UnhandledExceptionFunctor& onUnhandledException) {
    m_isRunning = true;
    while (m_isRunning) {
        std::optional<TaskId> finishedTask = runCurrentTask(onUnhandledException);
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

std::optional<TaskId> Worker::runCurrentTask(UnhandledExceptionFunctor& onUnhandledException) {
    std::unique_lock lock(m_mutex);
    if (!m_task.has_value()) {
        return std::nullopt;
    }

    TaskId currentTaskId = m_task->getId();
    try {
        m_task->run();
    } catch(std::exception& e) {
        if (onUnhandledException) {
            onUnhandledException(e);
        }
    }

    m_task = std::nullopt;
    return currentTaskId;
}

void Worker::waitForTask() {
    std::unique_lock lock(m_mutex);
    if (m_isRunning && !m_task.has_value()) {
        m_waitTaskCondittion.wait(lock);
    }
}

}
