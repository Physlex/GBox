module;
//
// #include <cstddef>
//
export module gbox.runtime:executor;
//
// import gbox.core;
// import gbox.ring;
// import gbox.funky;
//
// using fn::Yield;
// using memory::StaticRef;
// using result::Result;
// using ring::RingBuffer;
//
// class Error {
//   public:
//     enum class Kind {
//         Overrun,
//     };
// };
//
// /// This class implements a deferred execution evaluator, or "executor"
// ///
// /// ## Usage
// /// This class is intended to be invoked once per program runtime.
// ///
// /// TODO: DOCS, UPDATE
// template <std::size_t N>
// class Executor {
//   public:
//     Result<void, > run_once();
//
//   private:
//     // Each yield (when optimized) is effectively the size of a single pointer, mean
//     RingBuffer<StaticRef<Yield<void>>, N> queue_;
// };
