#include "executor.h"

namespace {
static thread_local std::vector<std::unique_ptr<kitten::Task>> s_collectedTasks;
static thread_local uint64_t s_workerIndex = -1;
static thread_local uint64_t s_taskIndex = 1;
}

namespace kitten {

Executor::Executor() {
    m_sheduler = std::make_unique<Sheduler>();
}

void Executor::start(int workersCount, std::function<void()> initialTask) {
    std::unique_lock lock(m_mutex);
    m_isRunning = true;
    std::vector<std::thread> workingThreads;
    for (int i = 0; i < workersCount; i++) {
        auto worker = std::make_unique<Worker>(this, i);
        m_workers.push_back(std::move(worker));
        if (i == 0) {
            auto task = std::make_unique<Task>(0, "Initial Task", std::move(initialTask));
            std::vector<std::unique_ptr<kitten::Task>> initialTasks;
            initialTasks.push_back(std::move(task));
            m_workers.back()->pushTasks(std::move(initialTasks));
        }
        std::thread workerThread([worker = m_workers.back().get(), index = i](){
            s_workerIndex = index;
            worker->start();
        });
        workingThreads.push_back(std::move(workerThread));
    }
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
    s_collectedTasks.push_back(std::make_unique<Task>(taskId, name, std::move(action)));
    return TaskBuilder(s_collectedTasks.back().get());
}

void Executor::onFreeWorker(Worker* worker) {
    std::unique_lock lock(m_mutex);

    if (m_freeWorkers.count(worker) > 0) {
        throw std::runtime_error("KittenTask: Free worker signaled about working.");
    }

    // TODO: unlock resources!!!!

    m_freeWorkers.insert(worker);
    collectTasks();
    findNextTasks(worker);

    if (m_workers.size() == m_freeWorkers.size()) {
        for (auto& worker : m_workers) {
            worker->stop();
        }
    }
}

void Executor::collectTasks() {
    std::vector<std::unique_ptr<kitten::Task>> collectedTasks;
    for (size_t i = 0; i < s_collectedTasks.size(); i++) {
        collectedTasks.push_back(std::move(s_collectedTasks[i]));
    }
    s_collectedTasks.clear();
    m_sheduler->pushTasks(std::move(collectedTasks));
}

void Executor::findNextTasks(Worker* freeWorker) {
    if (!m_isRunning) {
        return;
    }

    auto tasks = m_sheduler->popTasks();
    if (tasks.empty()) {
        // пробуем еще раз, но допускаем sleep
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
        tasks = m_sheduler->popTasks(&startTime);
        const auto now = std::chrono::high_resolution_clock::now();
        if (!tasks.empty() && startTime > now) {
            std::this_thread::sleep_for(startTime - now);
        }
    }

    while (freeWorker != nullptr) {
        // TODO: lock resources!!!!
        freeWorker->pushTasks(std::move(tasks));
        m_freeWorkers.erase(freeWorker);
        if (!m_freeWorkers.empty()) {
            freeWorker = *m_freeWorkers.begin();
            tasks = m_sheduler->popTasks();
            if  (tasks.empty()) {
                break;
            }
        } else {
            break;
        }
    }
}

}
