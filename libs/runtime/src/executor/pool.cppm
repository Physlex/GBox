module;

#include "gbox/core/types.hpp"

export module gbox.runtime:executor.pool;

import gbox.core.memory;
import gbox.core.ring;
import gbox.runtime:executor.mod;

namespace pool {

using memory::Pinned;

/// Define storage for tasks and operations
///
/// This is closest to the traditional "Scheduler" architecture, where we run through a
/// list of tasks and execute them.
///
/// However, unlike a naive implementation, any operation added to the pool is implied
to
    /// be ready for execution *immediately*, and will be run as though it is.
    ///
    /// ## Safety
    /// For safety reasons, an execution pool is pinned in memory at the place of
    invocation.
    /// It's storage type is it's invocation point.
    template <typename Sig, std::size_t C>
    class ExecutionPool : public Pinned {
  public:
    /// Generate a handle to the execution pool
    template <typename Derived>
    super::Scheduler<Derived, Sig> &&generate_handle() const;

    /// Runs the next task in the pool, reducing the count of tasks by 1
    super::Result<void> leak();

    /// Drains the storage by executing all collected tasks
    super::Result<void> drain();

  private:
    ring::Ring<Fn<Sig>, C> pool_;
};

template <typename Sig>
class ExecutionPoolView : public memory::Pinned {};

}  // namespace pool
