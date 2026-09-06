module;

#include <cstddef>

export module gbox.ring;

export import :policy;
export import :owned;

export namespace ring {

using Error = policy::Error;

template <typename T>
using Result = policy::Result<T>;

template <typename T, std::size_t C>
using RingBuffer = owned::OwnedStorage<T, C>;

}  // namespace ring
