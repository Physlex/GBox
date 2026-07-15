/** @file `examples/app-loop.c`
 *  @brief This file implements a simple runtime loop using the gbox core lib.
 */

#include <stdint.h>
#include <stdio.h>

#include <gbox/core/cell.hpp>
#include <gbox/core/result.hpp>
#include <gbox/runtime/executor.hpp>

using namespace gbox;

/// This should really be something that can be automatically "reinterpreted"
/// and type-specified
[[clang::annotate("task")]]
static inline void hello_msg(const char *msg) {
    printf("Hello, world! Message: %s\n", msg);
    return;
}

/// Meanwhile, this will be the original "main", which is generated via
/// attribute
[[clang::annotate("executor")]]
void user_main(executor::ExecutionChannel &&scheduler) {
    // That way task scheduling such as this is relatively simple.

    const auto task_1_res = scheduler.run_once([&]() { hello_msg("420"); });
    if (task_1_res.is_err()) {
        printf("ERROR: Task 0 failed to enqueue\n");
        return;
    }

    const auto task_2_res = scheduler.run_once([&]() { hello_msg("67"); });
    if (task_2_res.is_err()) {
        printf("ERROR: Task 1 failed to enqueue\n");
        return;
    }

    // And we don't really need to run any task, the one's queued just
    // "magically run"

    // TODO: Currently doesn't do anything except yield to the scheduler.
    //       Honestly, the runtime is currently: "async without await"
    return;
}

static cell::StaticCell<executor::ExecutionPool> POOL_CELL();

/// In theory, this will eventually be generated boilerplate for the "true"
/// entrypoint
int32_t main(int32_t argc, char **argv) {
    constexpr std::size_t MAX_TASKS = 1024;
    auto pool = POOL_CELL.init(std::move(executor::ExecutionPool<MAX_TASKS>()));
    auto scheduler = pool.generate_handle();
    scheduler.schedule([&scheduler]() { user_main(scheduler); });

    return pool.drain();
}
