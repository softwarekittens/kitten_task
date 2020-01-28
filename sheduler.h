#pragma once

#include "task.h"

namespace kitten {

class Sheduler {
public:
    Sheduler();

    void pushTasks(std::vector<std::unique_ptr<Task>> task);

    std::vector<std::unique_ptr<Task>> popTasks(std::chrono::time_point<std::chrono::high_resolution_clock>* sleepedUntil = nullptr);

    void lockResource(void* resource);

    void unlockResource(void* resource);

private:
};

}
