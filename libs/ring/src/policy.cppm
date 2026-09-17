module;

#include <cstddef>
#include <cstdint>
#include <utility>

export module gbox.ring:policy;
import gbox.func_ky;

using result::Result;

export namespace ring::policy {

/// Error aliases for the Ring container type
enum class Error : std::int8_t {
    /// Failed to push a value into the ring buffer
    Enqueue,

    /// Failed to dequeue a value from the ring buffer
    Dequeue
};

/// Result alias for the gbox ring container type
template <typename T>
using Result = result::Result<T, Error>;

/// Policy representing a "ring" type, which is a FIFO queue well-suited for embedded
template <class Self, typename T>
class Ring {
  public:
    Ring() = default;

    /// This method enqueues a single element to the end of a ring buffer
    ///
    /// ## Error
    /// Returns an Enqueue error on failure to push
    Result<void> push(const T value) {
        return static_cast<Self *>(this)->push_impl(std::move(value));
    }

    /// Return the last-recent element pushed onto the ring buffer
    ///
    /// ## Error
    /// Returns a Dequeue error on failure to pop
    Result<T> pop() { return static_cast<Self *>(this)->pop_impl(); };

    /// Checks whether the ring buffer has zero enqueued items
    [[nodiscard]] bool is_empty() const { return this->count_ == 0; }

    /// The current count of elements enqueued within the ringbuffer
    [[nodiscard]] std::size_t count() const { return this->count_; }

  protected:
    std::size_t reader_ = 0;
    std::size_t writer_ = 0;
    std::size_t count_ = 0;
};

}  // namespace ring::policy
