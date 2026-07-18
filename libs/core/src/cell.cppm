module;

#include <atomic>

export module gbox.core:cell;

//! This module defines a `Cell` type, which handles operating on memory in static space.
//!
//! Ideally, this make it simpler to determine when memory is being managed for static
//! memory systems, such as the executor.
//!
//! ## Note
//! A good way to think about cells in general are that they act like a kind of "slot" for
//! memory of a specified size, and contain metadata about whether that slot is being
//! used at all for analysis by both the compiler and runtime.
//!
//! They are technically slower to use then raw memory assignments, but that is exactly
//! the tradeoff we accept for the purposes of safe and robust software.

import :memory;
import :option;
import :result;

export namespace cell {

using option::None;
using option::Option;
using option::Some;

using result::Err;
using result::Ok;

/// Error definitions for the cell module
enum class Error {
    /// Despite memory being allocated for the cell's inner-type, the cell hasn't yet
    /// received data to fill the type.
    Empty,

    /// The cell has been previously initialized, and cannot be re-initialized
    Locked
};

template <typename T>
using Result = result::Result<T, Error>;

/// Defines a standard way of operating in static memory
template <typename T>
class StaticCell : public memory::Pinned {
  public:
    /// Constructs an unlocked `StaticCell`
    StaticCell() : inner_(None()) {};

    /// Initializes the cell exactly once
    ///
    /// If a cell is expected to have the same memory be re-allocated, prefer to "swap"
    /// the memory instead.
    ///
    /// ## Error
    /// If a cell has been initialized prior, then it is locked, and cannot be
    /// re-initialized.
    [[nodiscard]]
    inline Result<T &> init(T &&value) {
        [[unlikely]]
        if (this->inner_.is_some()) {
            return Err(Error::Locked);
        }

        this->inner_ = Some(std::move(value));
        return Ok(&this->inner_.assume_some());
    }

  protected:
    /// Forces the user to allocate memory in a way that never gets de-allocated, a'la
    /// static memory. (Or a leaked heap, effectively the same thing, but discouraged)
    ~StaticCell() = delete;

  private:
    // Atomic, meaning (ideally) safe across concurrency primitives at a basic level.
    // For true thread-safety, one would wrap this static cell type in a mutex.
    std::atomic<Option<T &&>> inner_;
};

}  // namespace cell
