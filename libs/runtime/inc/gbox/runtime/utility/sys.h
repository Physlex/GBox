#ifndef GBOX_CORE_UTILITY_PANIC_H_
#define GBOX_CORE_UTILITY_PANIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "gbox/core/types.hpp"

/** @def SYS_PANIC
 *  @brief system utility to wrap assertions.
 */
#define SYS_PANIC(ec)                           \
    printf("Failed with error code: %d\n", ec); \
    while (1);

#ifdef __cplusplus
};
#endif

#endif  // GBOX_CORE_UTILITY_PANIC_H_
