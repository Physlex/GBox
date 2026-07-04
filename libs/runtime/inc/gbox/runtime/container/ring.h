#ifndef SCHEDULER_CORE_CONTAINER_RING_H_
#define SCHEDULER_CORE_CONTAINER_RING_H_

/** @file `container/ring.h`
 *  @brief This file implements a simple byte-level ring buffer.
 *
 *         It allows queueing/dequeuing any memory type by copying it's
 *         byte-level representation.
 *
 *         The container uses a malloc'd implementation, as in the initial
 *         construction of the ring uses a memory allocation and hence, syscall.
 *
 *         To reduce overhead, the ring buffer is treated statically, meaning
 *         unless a second ring buffer is defined, the ring buffer provided has
 *         no built-in capability to change it's size.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "gbox/core/types.hpp"

struct ring;
typedef struct ring ring_t;

/** @fn int8_t ring_new(struct ring*, uint32_t, uint32_t)
 *  @brief This function allocates resources for the ring buffer.
 */
extern struct ring *ring_new(const uintptr_t size, const uintptr_t length);

/** @fn void ring_destroy(struct ring *)
 *  @brief This function releases the ring buffer resources.
 */
extern int8_t ring_destroy(struct ring **ctx);

/** @fn int8_t ring_enqueue(struct ring *, void *)
 *  @brief This function enqueues a single element to a ring buffer.
 *  @param[out] ctx The ring buffer, with writable addresses.
 *  @param[out] datum The base address of the type we are reading from.
 *  @result An error code as defined in `errors.h`
 */
extern int8_t ring_enqueue(struct ring *ctx, const void *datum);

/** @fn void *ring_dequeue(struct ring *)
 *  @brief This function dequeues a single element from a ring buffer.
 *  @param[out] ctx The ring buffer, with readable addresses.
 *  @param[out] datum The base address of the type we are reading to.
 *  @result An error code as defined in `errors.h`
 *  TODO: DOCS
 */
extern int8_t ring_dequeue(struct ring *ctx, void *datum);

/** @fn bool ring_is_empty(struct ring)
 *  @brief Checks whether the ring buffer is empty.
 *  @result true if empty, else, false.
 */
extern bool ring_is_empty(const struct ring *ctx);

/** @fn bool ring_is_full(struct ring)
 *  @brief Checks whether the ring buffer is full.
 *  @result true if full, else, false.
 */
extern bool ring_is_full(const struct ring *ctx);

/**
 * @defgroup ring_getters Ring Getters
 * @brief This group defines extractor functions for the ring type.
 * @{
 */

extern uintptr_t ring_read_ptr(const struct ring *ctx);
extern uintptr_t ring_write_ptr(const struct ring *ctx);
extern uintptr_t ring_capacity(const struct ring *ctx);

/** @} */

/** @defgroup ring_helpers Ring Helpers
 *  @brief This group defines a set of private ring helper functions.
 *  @{
 */

/** @fn void ring_read(const struct ring *)
 *  @brief read a single byte from the ring buffer into the read byte pointer.
 *
 *         Assumes that the maximum data representation of a given ring buffer
 *         is a singular byte.
 */
static void ring_read(struct ring *ctx, uint8_t *read_byte_ptr);

/** @fn void ring_write(struct ring *, const uint8_t)
 *  @brief read a single byte from the ring buffer into the read byte pointer.
 *
 *         Assumes that the maximum data representation of a given ring buffer
 *         is a singular byte.
 */
static void ring_write(struct ring *ctx, const uint8_t byte);

/** @} */

#ifdef __cplusplus
};
#endif

#endif  // SCHEDULER_CORE_CONTAINER_RING_H_
