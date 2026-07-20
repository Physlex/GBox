module;

export module gbox.runtime:executor;

import :executor.policy;
import :executor.pool;
import :executor.channel;

export namespace executor {

using Error = policy::Error;

template <typename T>
using Result = policy::Result<T>;

template <typename Sig>
using ExecutionChannel = channel::ExecutionChannel<Sig>;

}  // namespace executor
