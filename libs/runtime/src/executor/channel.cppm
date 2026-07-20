module;

//! This module defines the various event operations and structures for basic asynchronous
//! event generation-and-response.

export module gbox.runtime:executor.channel;

import :executor.policy;

export namespace executor::channel {

/// Implements a lightweight channel to the `ExecutorPool`
///
/// The only 'unique' part of the `ExecutionChannel` is the type-specified function
/// signature. In this case, the function signature doesn't allow type parameters or any
/// interesting return types.
///
/// In this manner, the execution channel is explicitly designed to be the 'dumb`
/// execution pool registrar.
///
/// Copying the execution channel is always intended to be lightweight, and will always
/// point to the same `ExecutionPool` regardless of how many channels live and die.
template <typename Sig>
class ExecutionChannel : public policy::Scheduler<ExecutionChannel<Sig>, Sig> {
    using Base = policy::Scheduler<ExecutionChannel<Sig>, Sig>;
    using typename Base::Task;

  protected:
    policy::Result<void> schedule_impl(Task task);

  private:
    // TODO: IMPLEMENT
    // ExecutionPoolView<void()> parent_;
};

}  // namespace executor::channel
