#ifndef GBOX_RUNTIME_EXECUTOR_MOD_HPP_
#define GBOX_RUNTIME_EXECUTOR_MOD_HPP_

//! This file defines the public API for the executor module, including channels, pools,
//! and the various utilities related.

#include "gbox/core/result.hpp"
#include "gbox/core/types.hpp"

namespace super {

/// Error definitions for the executor module
enum class Error {
    /// Failed to add new memory to the execution pool due to lack of space
    Overrun,
};

template <typename T>
using Result = result::Result<T, Error>;

/// Policy which defines a method of scheduling a `Derived` impl to an `ExecutionPool`
/// type.
///
/// Typically used on handles to the execution pool to enable sending typed function
/// pointers to a `ExecutionPool` of a given task type.
template <class Derived, typename Sig>
struct Scheduler {
    /// Task definition for this implementation of the `Scheduler` policy
    using Task = Fn<Sig>;

    /// Place a task into the executor, but don't execute said task
    inline Result<void> schedule(Task task) {
        return static_cast<Derived *>(this)->schedule_impl(task);
    }
};

}  // namespace super

#endif  // GBOX_RUNTIME_EXECUTOR_MOD_HPP_
