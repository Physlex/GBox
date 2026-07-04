#ifndef SCHEDULER_CORE_SCHED_H_
#define SCHEDULER_CORE_SCHED_H_

/** @file `scheduler.h`
 *  @brief TODO: DOCS
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "gbox/core/types.hpp"

//! @brief Forward declaration of ring type.
struct ring;

//! @brief Forward declaration of task type.
struct task;

/** @typedef scheduler_t
 *  @brief TODO: DOCS
 */
typedef struct scheduler scheduler_t;

/** @fn int8_t sched_init()
 *  @brief Initializes the scheduling subsystem.
 *  @return An error code, if the scheduler has already been initialized.
 */
extern int8_t sched_init(uintptr_t queue_length);

/** @fn scheduler_t::ErrorKind sched_enqueu(scheduler_t *, task_t *)
 *  @brief Enqueue a task to the scheduler for execution.
 *  TODO: DOCS
 */
extern int8_t sched_task(struct task *task);

/** @fn int8_t sched_start()
 *  @brief TODO: DOCS
 */
extern int8_t sched_run();

#ifdef __cplusplus
};
#endif

#endif  // SCHEDULER_CORE_SCHED_H_
