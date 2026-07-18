module;

export module gbox.runtime:executor;

import :executor.mod;
import :executor.pool;
import :executor.channel;

export namespace executor {
using Error = mod::Error;

template <typename T>
using Result = mod::Result<T>;
}  // namespace executor
