module;

#include <array>
#include <cstddef>
#include <type_traits>

export module gbox.ring:owned;

import gbox.core;
import gbox.func_ky;
import :policy;

export namespace ring::owned {

using result::Err;
using result::Ok;

/// Partial specialization of the RingStorage type into a capacity holding type
template <typename T, std::size_t C>
class OwnedStorage : public memory::MoveOnly, public policy::Ring<OwnedStorage<T, C>, T> {
    static_assert(C > 0, "Capacity must be greater than zero!");
    using Base = policy::Ring<OwnedStorage<T, C>, T>;

  public:
    /// Constructs an empty ringbuffer with capacity C and count 0
    OwnedStorage() : Base() {}

    /// Constructs a ringbuffer with capacity C, and count N, of type T
    ///
    /// Each element of the initializer list is propagated to the storage of the
    /// ringbuffer.
    ///
    /// ## Compilation Errors
    /// If the number of arguments supplied in the initializer list is greater then the
    /// capacity of the buffer, or the type assigned to one of the initializer values
    /// doesn't implicitly convert into the underlying Ring type, then we fail with
    /// a compilation error.
    template <typename... Args>
        requires(std::is_convertible_v<Args, T> && ...)
    OwnedStorage(Args... args) : Base() {
        static_assert(
            sizeof...(Args) <= C,
            "Ringbuffer cannot be populated with an initializer list of capacity greater "
            "than the buffer"
        );

        std::size_t index = 0;
        ((this->storage_[index++] = static_cast<T>(args)), ...);

        this->count_ = sizeof...(Args);
        this->writer_ = sizeof...(Args) % C;
    }

    policy::Result<T> pop_impl() {
        [[unlikely]]
        if (this->is_empty()) {
            return Err(policy::Error::Dequeue);
        }

        auto res = this->storage_[this->reader_];
        this->reader_ = (this->reader_ + 1) % C;
        --this->count_;

        return Ok(res);
    }

    policy::Result<void> push_impl(const T value) {
        [[unlikely]]
        if (this->count_ == C) {
            return Err(policy::Error::Enqueue);
        }

        this->storage_[this->writer_] = value;
        this->writer_ = (this->writer_ + 1) % C;
        ++this->count_;

        return Ok();
    }

  private:
    std::array<T, C> storage_;
};

}  // namespace ring::owned
