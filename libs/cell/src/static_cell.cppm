module;

//! This module implements the `StaticCell` type, a slot which a value is lifted into
//! exactly once and never leaves.
//!
//! A cell begins empty and is claimed by its first `init`, which moves the value into
//! destructor-free storage and hands back a `StaticRef` to where it settled. Every later
//! `init` is refused, so the reference the first caller holds names the only value the
//! cell will ever carry.
//!
//! ## Note
//! A cell performs no synchronization of its own. Two callers racing to `init` the same
//! cell may both observe it empty; wrap the cell where that is possible.

#include <gbox/core/types.hpp>
#include <utility>

export module gbox.cell:static_cell;

import gbox.core;
import gbox.funky;

using memory::Pinned;
using memory::Static;
using memory::StaticRef;
using option::None;
using option::Option;
using option::Some;
using result::Err;
using result::Ok;

export namespace static_cell {

/// Error definitions for the cell module
enum class Error : uint8_t {
    /// The cell has been previously initialized, and cannot be re-initialized
    Locked
};

template <typename T>
using Result = result::Result<T, Error>;

/// Lifts a value into static-lifetime storage, once.
///
/// # Example
/// ```cpp
/// auto &cell = StaticCell<Counter>::make<struct TickCounter>();
/// auto ref = cell.init(Counter(0)).assume_ok();
/// ref->tick();
/// ```
template <typename T>
class StaticCell final : public For<StaticCell<T>, Where<Pinned<Self>>> {
  public:
    /// Makes a cell and borrows it
    ///
    /// Every cell made this way has static storage duration, which is what makes the
    /// references it hands out honest. A cell cannot be made any other way, so one can
    /// never be built on a stack it would then outlive.
    ///
    /// ## Uniqueness
    /// The defaulted tag is a lambda type, and a lambda written at one call site shares
    /// its type with no other, so each call names a cell of its own.
    ///
    /// ## Warning
    /// Leave the tag alone. Naming one is a smell: it exists to keep cells apart, and the
    /// only reason to write one is to reach a cell a reference cannot be carried to.
    ///
    /// # Example
    /// ```cpp
    /// auto &cell = StaticCell<Counter>::make();
    /// ```
    template <auto Tag = [] {}>
    static StaticCell &make() {
        static StaticCell cell;
        return cell;
    }

    /// Claims the cell, moving the value into storage which never destroys it
    ///
    /// The reference returned points into the cell, so it stays valid for as long as the
    /// cell does. If a cell is expected to have the same memory be re-allocated, prefer
    /// to "swap" the memory instead.
    ///
    /// ## Error
    /// If a cell has been initialized prior, then it is locked, and cannot be
    /// re-initialized.
    [[nodiscard]]
    Result<StaticRef<T>> init(T &&value) {
        [[unlikely]]
        if (this->inner_.is_some()) {
            return Err(Error::Locked);
        }

        this->inner_ = Some(Static<T>(std::move(value)));
        return Ok(StaticRef<T>(this->inner_.as_mut()));
    }

  private:
    StaticCell() : inner_(None()) {}

    Option<Static<T>> inner_;
};

}  // namespace static_cell
