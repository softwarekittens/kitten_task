#include "task.h"
#include <kitten_logger/trace.h>

namespace kitten {

Task::Task(TaskId id, std::string_view name, std::function<void()> action)
    : m_id{id}
    , m_name{name}
    , m_action(std::move(action))
    , m_startTime{std::chrono::high_resolution_clock::now()}
{
}

void Task::run() {
    if (m_action) {
        TRACE_SCOPE;
        m_action();
    }
}

TaskId Task::getId() const {
    return m_id;
}

TaskBuilder& TaskBuilder::addDependTask(TaskId dependTask) {
    m_task->m_depends.push_back(dependTask);
    return *this;
}

TaskBuilder& TaskBuilder::addSharedResource(void* sharedResource) {
    m_task->m_resources.push_back(sharedResource);
    return *this;
}

TaskId TaskBuilder::getId() {
    return m_task->m_id;
}

}
