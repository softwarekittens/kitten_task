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
        {
            std::unique_lock lock(m_mutex);
            while (!m_tasks.empty()) {
                m_tasks.front()->run();
                m_tasks.pop();
            }
        }
        m_executor->onFreeWorker(this);
        waitForTask();
    }
}

void Worker::stop() {
    m_isRunning = false;
    m_waitTaskCondittion.notify_all();
}

void Worker::pushTasks(std::vector<std::unique_ptr<Task>> tasks) {
    std::unique_lock lock(m_mutex);
    for (size_t i = 0; i < tasks.size(); i++) {
        m_tasks.push(std::move(tasks[i]));
    }
    lock.unlock();
    m_waitTaskCondittion.notify_all();
}

void Worker::waitForTask() {
    std::unique_lock lock(m_mutex);
    if (m_isRunning && m_tasks.empty()) {
        m_waitTaskCondittion.wait(lock);
    }
}

}
