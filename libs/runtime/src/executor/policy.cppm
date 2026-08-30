module;

export module gbox.runtime:executor.policy;

import gbox.core;
import gbox.func_ky;

using func::FnOnce;

export namespace policy {

/// Error definitions for the executor module
enum class Error : uint8_t {
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
template <class Derived, typename Fn>
struct Scheduler {
    /// Task definition for this implementation of the `Scheduler` policy
    using Task = FnOnce<Fn>;

    /// Place a task into the executor, but don't execute said task
    Result<void> schedule(Task task) {
        return static_cast<Derived *>(this)->schedule_impl(task);
    }
};

}  // namespace policy
