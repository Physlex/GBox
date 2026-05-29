/** @file `examples/app-loop.c`
 *  @brief This file implements a simple runtime loop using the gbox core lib.
 */

#include <gbox/runtime/scheduler.h>
#include <gbox/runtime/tasks.h>
#include <gbox/runtime/utility/errors.h>
#include <stdint.h>
#include <stdio.h>

struct user_main_task_args {
    int32_t argc;
    char **argv;
};

/// This should really be something that can be automatically "reinterpreted"
/// and type-specified
[[clang::annotate("task")]]
static inline int32_t hello_msg(void *low_level_msg) {
    if (!low_level_msg) {
        return -EC_REQUIRES;
    }

    printf("Hello, %s!\n", (const char *)low_level_msg);

    return EC_SUCCESS;
}

/// Meanwhile, this will be the original "main", which is generated via
/// attribute
[[clang::annotate("executor")]]
int32_t user_main(void *args) {
    struct user_main_task_args args_actual = *(struct user_main_task_args *)args;

    task_t task_0 = simple_task_create(hello_msg, "Hello");
    task_t task_1 = simple_task_create(hello_msg, "World");

    // That way task scheduling such as this is relatively simple.

    if (sched_task(&task_0) < 0) {
        printf("ERROR: Task 0 failed to enqueue\n");
        return 1;
    }

    if (sched_task(&task_1) < 0) {
        printf("ERROR: Task 1 failed to enqueue\n");
        return 1;
    }

    // And we don't really need to run any task, the one's queued just
    // "magically run"

    // TODO: Currently doesn't do anything except yield to the scheduler.
    //       Honestly, the runtime is currently: "async without await"
    return 0;
}

/// In theory, this will eventually be generated boilerplate for the "true"
/// entrypoint
int32_t main(int32_t argc, char **argv) {
    uint32_t maximum_num_tasks = 100;
    sched_init(maximum_num_tasks);

    struct user_main_task_args user_main_args = {.argc = argc, .argv = argv};
    task_t user_main_task = simple_task_create(user_main, (void *)&user_main_args);
    sched_task(&user_main_task);

    // Ideally, we should be able to yield tasks early and return their result,
    // instead of ignore all return types.
    return -sched_run();
}
