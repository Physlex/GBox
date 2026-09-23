module;

// #include <gbox/core/types.hpp>
// #include <utility>

export module gbox.runtime:executor;

// import gbox.funky;
// import gbox.ring;
// import gbox.cell;

// using fn::Thunk;
// using option::Option;
// using result::Err;
// using result::Ok;
// using ring::RingBuffer;
// using static_ref::StaticRef;

// /// Copy/Cloneable error type for the executor class
// class Error {
//   public:
//     enum Value : uint8_t {
//         /// The executor failed to handle an erroneous edge case
//         Internal,
//         /// The API was missused by the operator
//         Misuse,
//     };

//     /// Implicitly constructed
//     Error(Value value) : value_(value) {}

//     /// Equality overload to check two representitive error types
//     [[nodiscard]] bool operator==(Error other) { return other.value_ == this->value_; }

//   private:
//     Value value_;
// };

// template <typename T>
// using Result = result::Result<T, Error>;

// /// Generic move-only callable which returns nothing.
// using Task = Thunk<void>;

// /// Number of tasks that can exist in the executor pool. When we eventually build the
// /// executor backend this will likely be configured through a constexpr.
// ///
// /// ## TODO:
// /// Is there a point in making this configurable later on?
// static const size_t POOL_CAP = 1024;

// /// Task executor for the RTOS
// ///
// /// ## Note
// /// Due to the way in which we orchestrated typing, a lot of our basic functionality is
// /// actually handled by the functional programming library.
// ///
// /// ## TODO:
// /// In future, we will be adding some more dynamics to the configuration of the
// executor,
// /// and allowing many different spawn sources to be use the executor as a "sink".
// ///
// class Executor {
//   public:
//     Executor() : _pool(RingBuffer<Task, POOL_CAP>()) {}

//     /// Immediately enqueue and run the task directly
//     ///
//     /// ## Usage
//     /// This member is only ever intended to be run once during `gbox::main`'s
//     /// initialization sequence, and is specifically used to run the user's main
//     function.
//     ///
//     /// Although it can be used outside of that context, it's not reccomended, since it
//     is
//     /// nearly the equivalent of invoking the function directly.
//     static void run_once(Task &&task) { std::move(task)(); }

//     Result<void> enqueue(Task &&task) {
//         auto res = this->pool_.push(std::move(task));
//         if (res.is_err()) {
//             return Err(Error::Internal);
//         }
//     }

//   private:
//     RingBuffer<Task, POOL_CAP> pool_;
// };

// /// Spawn source handler
// ///
// /// Uses an internal static reference to the executor deriving this source
// class SpawnSource {
//   public:
//     static SpawnSource make(StaticRef<Executor> executor_ref) {
//         return SpawnSource(std::forward<StaticRef<Executor>>(executor_ref));
//     }

//     /// Spawn a task and enqueue it to the executor
//     ///
//     /// ## Error
//     /// If the executor fails to enqueue, then we return an `Internal` error.
//     Result<void> spawn(Task &&task) {
//         auto res = this->executor_ref_->enqueue(std::move(task));
//         if (res.is_err()) {
//             return Err(Error::Internal);
//         }

//         return Ok<void>();
//     };

//   protected:
//     SpawnSource(StaticRef<Executor> executor_ref)
//         : executor_ref_(std::move(executor_ref)) {}

//   private:
//     StaticRef<Executor> executor_ref_;
// };
