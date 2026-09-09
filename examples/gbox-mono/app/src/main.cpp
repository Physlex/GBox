/** @file `examples/gbox-mono/app/src/main.cpp`
 *  @brief This file demonstrates consuming gbox as a package from a downstream project.
 *
 *  The two `import` declarations are what this example exists to prove. Neither module is
 *  built here: both are compiled from interface units the gbox package installed, reached
 *  through `find_package(gbox)`.
 */

#include <stdint.h>
#include <stdio.h>

#include <utility>

import gbox.core;
import gbox.func_ky;
import gbox.runtime;

using result::Result;

/// Error cases for the demonstration below.
enum class Error : uint8_t {
    /// Doubling was asked for on a value that cannot be doubled.
    Negative
};

/// Doubles a value, or reports why it could not.
static result::Result<int32_t, Error> doubled(int32_t value) {
    if (value < 0) {
        return result::Err(Error::Negative);
    }

    return result::Ok(value * 2);
}

int32_t main(int32_t argc, char **argv) {
    auto valid = doubled(21);
    auto erroneous = doubled(-1);

    printf("doubled: %d\n", std::move(valid).assume_ok());
    printf("negative is erroneous: %s\n", erroneous.is_err() ? "yes" : "no");

    return 0;
}

// TODO: Restore the executor demonstration below once the runtime supports it. The pool
// is currently a skeleton: `ExecutionPool::generate_handle`, `ExecutionPool::drain`, and
// `ExecutionChannel::schedule_impl` are declared with no definition, `ExecutionPool` is
// not exported from `gbox.runtime`, and it takes a task signature alongside its capacity.
//
// /// This should really be something that can be automatically "reinterpreted"
// /// and type-specified
// [[clang::annotate("task")]]
// static inline void hello_msg(const char *msg) {
//     printf("Hello, world! Message: %s\n", msg);
//     return;
// }
//
// /// Meanwhile, this will be the original "main", which is generated via
// /// attribute
// [[clang::annotate("executor")]]
// void user_main(executor::ExecutionChannel &&scheduler) {
//     // That way task scheduling such as this is relatively simple.
//
//     const auto task_1_res = scheduler.run_once([&]() { hello_msg("420"); });
//     if (task_1_res.is_err()) {
//         printf("ERROR: Task 0 failed to enqueue\n");
//         return;
//     }
//
//     const auto task_2_res = scheduler.run_once([&]() { hello_msg("67"); });
//     if (task_2_res.is_err()) {
//         printf("ERROR: Task 1 failed to enqueue\n");
//         return;
//     }
//
//     // And we don't really need to run any task, the one's queued just
//     // "magically run"
//
//     // TODO: Currently doesn't do anything except yield to the scheduler.
//     //       Honestly, the runtime is currently: "async without await"
//     return;
// }
//
// static cell::StaticCell<executor::ExecutionPool> POOL_CELL();
//
// /// In theory, this will eventually be generated boilerplate for the "true"
// /// entrypoint
// int32_t main(int32_t argc, char **argv) {
//     constexpr std::size_t MAX_TASKS = 1024;
//     auto pool = POOL_CELL.init(std::move(executor::ExecutionPool<MAX_TASKS>()));
//     auto scheduler = pool.generate_handle();
//     scheduler.schedule([&scheduler]() { user_main(scheduler); });
//
//     return pool.drain();
// }
