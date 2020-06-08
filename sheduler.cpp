#include "sheduler.h"
#include <kittens_trace/trace.h>

namespace kitten {

Sheduler::Sheduler(size_t workersCount)
    : m_workersCount{workersCount}
{
}

void Sheduler::pushTask(Task task) {
    m_waitingTasks.push_back(std::move(task));
}

void Sheduler::markTaskAsFinished(TaskId taskId) {
    const auto resources = m_runningTasks.at(taskId);
    for (auto resource : resources) {
        if (m_lockedResources.count(resource) == 0) {
            throw std::runtime_error("Fuuuuuu");
        }
        m_lockedResources.erase(resource);
    }
    m_runningTasks.erase(taskId);
}

std::optional<Task> Sheduler::popTask(std::chrono::time_point<std::chrono::high_resolution_clock>* sleepedUntil) {
    TRACE_SCOPE;

    std::vector<std::list<Task>::iterator> readyTasks;
    for (auto it = m_waitingTasks.begin(); it != m_waitingTasks.end(); it++) {
        if (isTaskReady(*it)) {
            readyTasks.push_back(it);
        }
    }
    if (readyTasks.empty()) {
        return std::nullopt;
    }

    // выбираем лучшую таску. Пока что лучшей назовем ту, которая дольше всего ждет в очереди
    std::list<Task>::iterator bestTask = readyTasks.front();
    for (auto it : readyTasks) {
        if (bestTask->m_startTime > it->m_startTime) {
            bestTask = it;
        }
    }

    const auto now = std::chrono::high_resolution_clock::now();
    if (bestTask->m_startTime > now) {
        // таска готова к выполнению, но ей нужен sleep. Позволим сделать sleep только в том случае, когда в других потоках ничего не выполняется
        if (m_runningTasks.empty()) {
            // чтож, делать больше нехер - ждем-с
            TRACE_NAMED_SCOPE("Executor Sleep");
            std::this_thread::sleep_for(bestTask->m_startTime - now);
        } else {
            // надеемся, что таски в других потоках не будут слишком долгими и еще раз триггернут bestTask когда настанет ее время
            return std::nullopt;
        }
    }

    std::optional<Task> result = std::move(*bestTask);
    m_waitingTasks.erase(bestTask);
    return std::move(result);
}

void Sheduler::markTaskAsStarted(Task& task) {
    // лочим ресурсы таски
    for (auto resource : task.m_resources) {
        if (m_lockedResources.count(resource) > 0) {
            throw std::runtime_error("Fuuuuuu");
        }
        m_lockedResources.insert(resource);
    }
    m_runningTasks.insert({ task.getId(), task.m_resources });
}

bool Sheduler::isTaskReady(Task& task) const {
    for (const auto& dep : m_waitingTasks) {
        if (dep.m_id == task.m_id) {
            continue;
        }

        for (auto id : task.m_depends) {
            if (id == dep.m_id) {
                // не можем запустить таску потому что она зависит от таски, которая ждет выполнения
                return false;
            }
        }
    }

    for (const auto& [taskId, resources] : m_runningTasks) {
        for (auto id : task.m_depends) {
            if (id == taskId) {
                // не можем запустить таску потому что она зависит от таски, которая еще выполняется
                return false;
            }
        }

        for (auto resource1 : resources) {
            for (auto resource2 : task.m_resources) {
                if (resource1 == resource2) {
                    // не можем запустить таску потому что ей требуется ресурс, залоченный другой таской
                    return false;
                }
            }
        }
    }

    return true;
}

}
