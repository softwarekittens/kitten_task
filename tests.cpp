#include "tests.h"
#include <kitten_logger/trace.h>
#include "executor.h"

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

namespace kitten {

void testTrace1() {
    struct Runner {
        void run() {
            TRACE_SCOPE;
            std::this_thread::sleep_for(10ms);
            runInner();
            std::this_thread::sleep_for(10ms);
            runInner();
        }

        void runInner() {
            TRACE_SCOPE;
            std::this_thread::sleep_for(10ms);
        }
    };

    std::thread thread1([](){
        Runner runner;
        runner.run();
    });
    std::thread thread2([](){
        Runner runner;
        runner.run();
    });
    std::thread thread3([](){
        Runner runner;
        runner.run();
    });

    thread1.join();
    thread2.join();
    thread3.join();
    
    saveCollectedTracing("build/kitten_trace_test_1.json");
}

void testExecutor1() {
    Executor executor;
    executor.start(1, []() {
        std::this_thread::sleep_for(5ms);
    }, [](auto&){ });
    saveCollectedTracing("build/kitten_executor_test_1.json");
}

void testExecutor2() {
    Executor executor;
    executor.start(4, []() {
        std::this_thread::sleep_for(5ms);

        Executor::addTask("Task 1", [](){
            std::this_thread::sleep_for(5ms);
        }).addDelay(10ms);

        Executor::addTask("Task 1", [](){
            std::this_thread::sleep_for(5ms);
        }).addDelay(20ms);

        Executor::addTask("Task 1", [](){
            std::this_thread::sleep_for(5ms);
        }).addDelay(30ms);
    }, [](auto&){ });
    saveCollectedTracing("build/kitten_executor_test_2.json");
}

void testExecutor3() {
    Executor executor;
    executor.start(4, []() {
        std::this_thread::sleep_for(5ms);

        for (int i = 0; i < 30; i++) {
            Executor::addTask("Task", [](){
                std::this_thread::sleep_for(5ms);
            });
        }
    }, [](auto&){ });
    saveCollectedTracing("build/kitten_executor_test_3.json");
}

}
