module;

#include <cstddef>

export module gbox.runtime:executor.pool;

import gbox.func_ky;
import gbox.core;
import gbox.ring;

import :executor.policy;

using func::FnOnce;
using memory::Pinned;
using ring::RingBuffer;

export namespace pool {

/// Backend for the executor, can be configured to work on any arbitrary persistence layer
template <typename Sig, std::size_t C>
class ExecutionPool : public Pinned {
  public:
    /// Default constructor for the execution pool
    ExecutionPool<Sig, C>() : pool_(RingBuffer<FnOnce<Sig>, C>()) {}

    /// Construct a pool with a pre-allocated buffer
    ExecutionPool<Sig, C>(RingBuffer<FnOnce<Sig>, C> pool) : pool_(pool) {}

    /// Runs the next task in the pool, reducing the count of tasks by 1
    policy::Result<void> leak() { this->pool_.pop(); }

    /// Drains the storage by executing all collected tasks
    policy::Result<void> drain() {}

  private:
    RingBuffer<FnOnce<Sig>, C> pool_;
};

template <typename Sig>
class ExecutionPoolView : public memory::Pinned {};

}  // namespace pool
