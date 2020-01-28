#include "task.h"

namespace kitten {

Task::Task(TaskId id, std::string_view name, std::function<void()> action)
    : m_id{id}
    , m_name{name}
    , m_action(std::move(action))
    , m_startTime{std::chrono::high_resolution_clock::now()}
{
}

void Task::run() {
}

TaskBuilder& TaskBuilder::addDependTask(TaskId dependTask) {
    return *this;
}

TaskBuilder& TaskBuilder::addSharedResource(void* sharedResource) {
    return *this;
}

TaskId TaskBuilder::getId() {
    return m_task->m_id;
}

}
