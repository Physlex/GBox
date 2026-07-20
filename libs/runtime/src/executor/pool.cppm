module;

#include "gbox/core/types.hpp"

export module gbox.runtime:executor.pool;

// import gbox.core;
// import :executor.policy;

// export namespace executor::pool {

// using memory::Pinned;

// template <typename Sig, std::size_t C>
// class ExecutionPool : public Pinned {
//   public:
//     /// Generate a handle to the execution pool
//     template <typename Derived>
//     policy::Scheduler<Derived, Sig> &&generate_handle() const;

//     /// Runs the next task in the pool, reducing the count of tasks by 1
//     policy::Result<void> leak();

//     /// Drains the storage by executing all collected tasks
//     policy::Result<void> drain();

//   private:
//     ring::RingBuffer<Fn<Sig>, C> pool_;
// };

// template <typename Sig>
// class ExecutionPoolView : public memory::Pinned {};

// }  // namespace executor::pool
