#pragma once

#include "task.h"
#include "worker.h"
#include "sheduler.h"

#include <list>
#include <set>
#include <thread>
#include <mutex>
#include <functional>

namespace kitten {

class Executor {
public:
    Executor();

    // Запустить executor. Будет создано workersCount потоков и на одном потоке будет запущена начальная таска initialTask.
    void start(size_t workersCount, std::function<void()> initialTask, UnhandledExceptionFunctor onUnhandledException);

    void stop();

    // Добавить таску. Метод должен вызываться только из тасок
    // action должен уметь перемещаться (std::move)
    static TaskBuilder addTask(std::string_view name, std::function<void()> action);

    friend class Worker;
private:
    std::vector<std::thread> initWorkers();

    std::thread startWorker();

    void onFreeWorker(Worker* worker, TaskId finishedTaskId);

    void collectTasks();

    void runNextTasks();

private:
    std::mutex m_mutex;
    bool m_isRunning = false;
    std::vector<std::unique_ptr<Worker>> m_workers;
    std::set<Worker*> m_freeWorkers;
    std::unique_ptr<Sheduler> m_sheduler;
};

}
