#ifndef SCHEDULER_CORE_TASKS_H_
#define SCHEDULER_CORE_TASKS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "gbox/runtime/scheduler.h"

/** @} */

/** @defgroup type_aliases
 *  @brief This group defines various generic type aliases that better define
 *         developer intent.
 *  @{
 */

/** @typedef int32_t (*gen_callback_ptr_t)(void*)
 *  @brief Generic callback alias.
 *
 *         This function alias defines a better semantic interpretation of the
 *         concept of a fully generic function definition.
 *
 *         A generic callback is expected to be one-shot. That is, not
 *         returning a structure to be seized by some controller. Instead, it
 *         is expected to die after firing.
 */
typedef int32_t (*gen_funcptr_t)(void *);

/** @} */

/** @file `tasks.h`
 *  @brief This file defines the various task definitions used by the runtime.
 */

/** @enum task_state
 *  @typedef task_state_k
 *  @brief This enumerator defines various return states of the `task_poll`
 *         operation.
 */
typedef enum task_state {
    /** @memberof enum task_state
     *  @brief TS_READY is returned from `poll` whenever a task is ready to
     *         execute.
     *
     *         A ready task is the only task state in which the task is
     *         interested in being run.
     */
    TS_READY,

    /** @memberof enum task_state
     *  @brief TS_WAITING is returned from `poll` whenever a task is unable to
     *         execute, or is otherwise waiting for further completion.
     *
     *         Typically a waiting task is not cleaned up, nor is it ran.
     */
    TS_WAITING,

    /** @memberof enum task_state
     *  @brief TS_DONE is return from `poll` whenever a task is no longer
     *         interested in being run.
     *
     *         It is at this point that a task can be cleaned up and/or deleted.
     */
    TS_DONE
} task_state_k;

struct task_if;

/** @typedef poll_callback_ptr_t
 *  @brief TODO: DOCS
 */
typedef int8_t (*poll_callback_ptr_t)(struct task *);

/** @struct task_if
 *  @typedef task_vt_t
 *  @brief Virtual interface for tasks.
 */
typedef struct task_if {
    gen_callback_ptr_t run;
    poll_callback_ptr_t poll;
} task_vt_t;

/** @struct task
 *  @brief Simple task definition. Wraps a generic callback pointer and tracks
 *         state.
 *
 *  TODO: DOCS
 */
typedef struct task {
    task_vt_t interface;
    void *args;
    task_state_k state;
} task_t;

/** @typedef task_t
 *  @brief Alias to a generic task. Simplest possible task defintion.
 */
typedef struct task task_t;

/** @fn struct task simple_task_create(gen_callback_ptr_t, void *)
 *  @brief TODO: DOCS
 */
extern struct task simple_task_create(gen_callback_ptr_t run, void *args);

/** @fn int32_t task_run(struct task *)
 *  @brief Runs the simple task, then marks the task as completed.
 */
extern int32_t task_run(struct task *ctx);

/** @fn task_poll
 *  @brief Polls a task to query if it is ready.
 *  @return positive task_state_k. On error, negative error code.
 */
extern int8_t task_poll(struct task *task);

/** @fn uinptr_t task_size()
 *  @brief Extractor for number of bytes in a simple task struct.
 */
extern uintptr_t task_size();

/** @fn task_if(struct task_if *)
 *  @brief Extractor for task interface.
 */
extern const struct task_if task_if(struct task *task);

/** @fn struct task task_create(const task_vt_t interface, void *args)
 *  @brief Simple task creation utility. Takes a set of args and 'hooks' into
 *         scheduler.
 *
 *  @param[in] task_if Interface definition for a simple task.
 *  @param args The arguments provided on startup to the simple task.
 *
 *  @return A constructed simple task.
 *
 *  TODO: DOCS
 */
static struct task task_create(const task_vt_t interface, void *args);

#ifdef __cplusplus
};
#endif

#endif  // SCHEDULER_CORE_TASKS_H_
