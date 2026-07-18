module;

#include <cstddef>

export module gbox.core:ring;

import :ring.mod;
import :ring.owned;
import :ring.viewed;

export namespace ring {
    using Error = mod::Error;

    template<typename T, std::size_t C>
    using RingBuffer = owned::OwnedStorage<T, C>;

    template<typename T>
    using RingView = viewed::ViewedStorage<T>;
}  // namespace ring
