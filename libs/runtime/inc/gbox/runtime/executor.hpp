#ifndef GBOX_RUNTIME_EXECUTOR_HPP_
#define GBOX_RUNTIME_EXECUTOR_HPP_

//! This file exports the public api of the executor module, wrapping it into a modular
//! namespace.

namespace executor {
#include "executor/channel.hpp"
#include "executor/mod.hpp"
#include "executor/pool.hpp"
}  // namespace executor

#endif  // GBOX_RUNTIME_EXECUTOR_HPP_
