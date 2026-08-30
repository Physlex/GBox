module;

#include <cstddef>

export module gbox.ring;

export import :policy;
export import :owned;
export import :viewed;

export namespace ring {

using Error = policy::Error;

template <typename T>
using Result = policy::Result<T>;

template <typename T, std::size_t C>
using RingBuffer = owned::OwnedStorage<T, C>;

template <typename T>
using RingView = viewed::ViewedStorage<T>;

}  // namespace ring
