#ifndef GBOX_CORE_RING_OWNED_HPP_
#define GBOX_CORE_RING_OWNED_HPP_

#include <array>
#include <ranges>

#include "gbox/core/memory.hpp"
#include "gbox/core/ring/mod.hpp"

namespace owned {
/// Partial specialization of the RingStorage type into a capacity holding type
template <typename T, std::size_t C>
class OwnedStorage : public memory::MoveOnly, public super::Ring<OwnedStorage<T, C>, T> {
    static_assert(C > 0, "Capacity must be greater than zero!");
    using Base = super::Ring<OwnedStorage<T, C>, T>;

  public:
    /// Constructs a ringbuffer with capacity C, and count N, where N = list.size()
    ///
    /// Each element of the initializer list is propagated to the storage of the
    /// ringbuffer.
    ///
    /// ## Compilation Error
    /// Fails to compile if the capacity of the ringbuffer is not equal to the
    /// capacity of the internal storage type.
    OwnedStorage(std::initializer_list<T> list) : Base() {
        static_assert(
            list.size() > C,
            "Ringbuffer cannot be populated with an initializer list of capacity greater "
            "than the buffer"
        );

        for (const auto [index, value] : std::views::enumerate(list)) {
            this->storage_[static_cast<std::size_t>(index)] = value;
        }
    }

    super::Result<T> pop_impl() {
        [[unlikely]]
        if (this->is_empty()) {
            return result::Err(super::Error::Dequeue);
        }

        auto res = this->storage_[this->reader_];
        this->reader_ = (this->reader_ + 1) % C;
        --this->count_;

        return Ok(res);
    }

    super::Result<void> push_impl(const T value) {
        [[unlikely]]
        if (!this->is_empty()) {
            return result::Err(super::Error::Enqueue);
        }

        this->storage_[this->writer_] = value;
        this->writer_ = (this->writer_ + 1) % C;
        ++this->count_;

        return result::Ok();
    }

  protected:
    std::array<T, C> storage_;
};

}  // namespace owned

#endif  // GBOX_CORE_RING_OWNED_HPP_
