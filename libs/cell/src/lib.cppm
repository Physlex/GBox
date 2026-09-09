//! This module defines the cell container types, used to specify stack-initialized but
//! globally specified memory cells.
//!
//! A good way to think about cells in general are that they act like a kind of "slot" for
//! memory of a specified size, and contain metadata about whether that slot is being used
//! at all for analysis by both the compiler and runtime.
//!
//! They are technically slower to use then raw memory assignments, but that is exactly
//! the tradeoff we accept for the purposes of safe and robust software.
//!
//! ## Warning
//! `StaticCell` is the entry point to this module. A `Static` is storage which never
//! destroys what it holds, and constructing one directly is discouraged: on its own it
//! suppresses destruction without granting the lifetime that makes suppression safe, so a
//! `Static` built on the stack abandons its value when the frame goes away, leaking it
//! rather than keeping it alive.
//!
//! A cell holds that storage for as long as it lives and hands back a `StaticRef` to the
//! value it settled, which is what turns suppression into a lifetime the compiler can
//! audit. Reach for a cell, and take the reference it gives you.

export module gbox.cell;

export import :static_cell;
export import :static_ref;
export import :static_t;

export namespace cell {

using static_cell::Error;
using static_cell::StaticCell;
using static_ref::StaticRef;
using static_t::Static;

template <typename T>
using Result = static_cell::Result<T>;

}  // namespace cell
