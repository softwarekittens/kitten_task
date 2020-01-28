#include "sheduler.h"

namespace kitten {

Sheduler::Sheduler() {

}

void Sheduler::pushTasks(std::vector<std::unique_ptr<Task>> tasks) {

}

std::vector<std::unique_ptr<Task>> Sheduler::popTasks(std::chrono::time_point<std::chrono::high_resolution_clock>* sleepedUntil) {
    return {};
}

void Sheduler::lockResource(void* resource) {

}

void Sheduler::unlockResource(void* resource) {

}

}
