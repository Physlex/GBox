#ifndef GBOX_CORE_MEMORY_HPP_
#define GBOX_CORE_MEMORY_HPP_

//! This module implements a memory constraining types for classes and structs
//!
//! They enforce certain memory behaviours and interactions for the developer.
//!
//! The following policies are included:
//!
//!   * MoveOnly -- Deletes implicit copy and assignment operators, enforcing the
//!     type to be moved using `std::move` instead.
//!
//!   * Pinned -- Deletes all move, assignment, and copy operations. Forces the memory
//!     to be defineed in exactly one spot, or otherwise referenced instead.

namespace memory {

class MoveOnly {
    MoveOnly() = default;
    MoveOnly(const MoveOnly &) = delete;
    MoveOnly &operator=(const MoveOnly &) = delete;
    MoveOnly(MoveOnly &&) noexcept = default;
    MoveOnly &operator=(MoveOnly &&) noexcept = default;
};

class Pinned {
    Pinned() = default;
    Pinned(const Pinned &) = delete;
    Pinned &operator=(const Pinned &) = delete;

  protected:
    ~Pinned() = default;
};

}  // namespace memory

#endif  // GBOX_CORE_MEMORY_HPP_
