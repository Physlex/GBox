module;

#include <array>
#include <cstddef>
#include <ranges>

export module gbox.core:ring.owned;

import :memory;
import :result;
import :ring.mod;

export namespace owned {

using result::Err;
using result::Ok;

/// Partial specialization of the RingStorage type into a capacity holding type
template <typename T, std::size_t C>
class OwnedStorage : public memory::MoveOnly, public mod::Ring<OwnedStorage<T, C>, T> {
    static_assert(C > 0, "Capacity must be greater than zero!");
    using Base = mod::Ring<OwnedStorage<T, C>, T>;

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

    mod::Result<T> pop_impl() {
        [[unlikely]]
        if (this->is_empty()) {
            return Err(mod::Error::Dequeue);
        }

        auto res = this->storage_[this->reader_];
        this->reader_ = (this->reader_ + 1) % C;
        --this->count_;

        return Ok(res);
    }

    mod::Result<void> push_impl(const T value) {
        [[unlikely]]
        if (!this->is_empty()) {
            return Err(mod::Error::Enqueue);
        }

        this->storage_[this->writer_] = value;
        this->writer_ = (this->writer_ + 1) % C;
        ++this->count_;

        return Ok();
    }

  protected:
    std::array<T, C> storage_;
};

}  // namespace owned
