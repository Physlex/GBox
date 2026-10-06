module;

#include <cstddef>
#include <utility>

export module gbox.runtime:executor;

import gbox.core;
import gbox.ring;
import gbox.funky;

using memory::StaticRef;
using ring::RingBuffer;

using fn::Yield;
using option::Option;
using result::Err;
using result::Ok;
using result::Result;

/// This class implements a deferred execution evaluator, or "executor"
///
/// ## Usage
/// This class is intended to be invoked once per program runtime.
///
/// TODO: DOCS, UPDATE
template <std::size_t N>
class Executor {
  private:
    // Each yield (when optimized) is the size of a single pointer
    //
    // Assuming the yield lives for the duration of the program and is trivially
    // destructible, that means that we can store a gaggle of these without concern.
    using Task = StaticRef<Yield<void>>;

  public:
    /// The executor enqueus the task into it's internal queue, taking ownership
    ///
    /// ## Note
    /// The task doesn't run once queued inside the ringbuffer, instead it is deferred
    /// until an invocation of the [`run`] member.
    constexpr Result<void, Task> enqueue(Task &&task) {
        [[unlikely]]
        if (this->queue_.is_full()) {
            return Err(std::move(task));
        }

        this->queue_.push(std::move(task));

        return Ok();
    }

    /// The executor immediately runs the provided task in-place
    constexpr void run_once(Task &&task) noexcept { std::move(task)->yield(); }

    /// The executor runs until the queue is empty, and then returns immediately
    constexpr void run_to_completion() noexcept {
        for (auto task = this->queue_.pop(); task.is_ok(); task = this->queue_.pop()) {
            this->run_once(std::move(task).assume_ok());
        }
    }

  private:
    RingBuffer<Task, N> queue_;
};
