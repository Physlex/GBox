#ifndef GBOX_CORE_RING_MOD_HPP_
#define GBOX_CORE_RING_MOD_HPP_

#include <cstddef>

#include "gbox/core/result.hpp"

namespace super {

/// Error aliases for the Ring container type
enum class Error {
    /// Failed to push a value into the ring buffer
    Enqueue,

    /// Failed to dequeue a value from the ring buffer
    Dequeue
};

/// Result alias for the gbox ring container type
template <typename T>
using Result = result::Result<T, Error>;

/// Policy representing a "ring" type, which is a FIFO queue well-suited for embedded
template <class Derived, typename T>
class Ring {
  public:
    // Constructs a ringbuffer with capacity C, and count 0
    Ring() : reader_(0), writer_(0), count_(0) {}

    /// This method enqueues a single element to the end of a ring buffer
    ///
    /// ## Error
    /// Returns an Enqueue error on failure to push
    inline Result<void> push(const T value) {
        return static_cast<Derived *>(this)->push(std::move(value));
    }

    /// Return the last-recent element pushed onto the ring buffer
    ///
    /// ## Error
    /// Returns a Dequeue error on failure to pop
    inline Result<T> pop() { return static_cast<Derived *>(this)->pop_impl(); };

    /// Checks whether the ring buffer has zero enqueued items
    ///
    /// ## Return
    /// If true, then empty. Else, false.
    inline bool is_empty() const { return this->count_ == 0; }

    /// The current count of elements enqueued within the ringbuffer
    std::size_t count() const;

  protected:
    std::size_t reader_ = 0;
    std::size_t writer_ = 0;
    std::size_t count_ = 0;
};

}  // namespace super

#endif  // GBOX_CORE_RING_MOD_HPP_
