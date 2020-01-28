#include "executor.h"
#include <kitten_logger/trace.h>
#include <deque>

namespace {
static thread_local std::deque<kitten::Task> s_collectedTasks;
static thread_local uint64_t s_workerIndex = -1;
static thread_local uint64_t s_taskIndex = 2;
}

namespace kitten {

Executor::Executor() {
}

void Executor::start(size_t workersCount, std::function<void()> initialTask) {
    std::unique_lock lock(m_mutex);
    m_sheduler = std::make_unique<Sheduler>(workersCount);
    m_isRunning = true;
    std::vector<std::thread> workingThreads;
    for (size_t i = 0; i < workersCount; i++) {
        m_workers.push_back(std::make_unique<Worker>(this, i));
        if (i == 0) {
            Task task(1, "Initial Task", std::move(initialTask));
            m_sheduler->markTaskAsStarted(task);
            m_workers.back()->pushTask(std::move(task));
        } else {
            m_freeWorkers.insert(m_workers.back().get());
        }
        std::thread workerThread([worker = m_workers.back().get(), index = i](){
            s_workerIndex = index;
            worker->start();
        });
        workingThreads.push_back(std::move(workerThread));
    }
    lock.unlock();
    for (auto& workingThread : workingThreads) {
        workingThread.join();
    }
}

void Executor::stop() {
    std::unique_lock lock(m_mutex);
    m_isRunning = false;
}

TaskBuilder Executor::addTask(std::string_view name, std::function<void()> action) {
    TaskId taskId = s_taskIndex + (s_workerIndex << 32);
    s_taskIndex++;
    Task task(taskId, name, std::move(action));
    s_collectedTasks.push_back(std::move(task));
    return TaskBuilder(&s_collectedTasks.back());
}

void Executor::onFreeWorker(Worker* worker, TaskId finishedTaskId) {
    std::unique_lock lock(m_mutex);
    TRACE_SCOPE;

    if (m_freeWorkers.count(worker) > 0) {
        throw std::runtime_error("KittenTask: Free worker signaled about working.");
    }

    m_sheduler->markTaskAsFinished(finishedTaskId);
    m_freeWorkers.insert(worker);
    collectTasks();
    runNextTasks();

    if (m_workers.size() == m_freeWorkers.size()) {
        for (auto& worker : m_workers) {
            worker->stop();
        }
    }
}

void Executor::collectTasks() {
    std::vector<std::unique_ptr<kitten::Task>> collectedTasks;
    for (size_t i = 0; i < s_collectedTasks.size(); i++) {
        m_sheduler->pushTask(std::move(s_collectedTasks[i]));
    }
    s_collectedTasks.clear();
}

void Executor::runNextTasks() {
    if (!m_isRunning) {
        return;
    }

    while (true) {
        if (m_freeWorkers.empty()) {
            return;
        }
        auto freeWorker = *m_freeWorkers.begin();
        auto task = m_sheduler->popTask();
        if (!task.has_value()) {
            return;
        }

        m_sheduler->markTaskAsStarted(*task);
        freeWorker->pushTask(std::move(task.value()));
        m_freeWorkers.erase(freeWorker);
    }
}

}
