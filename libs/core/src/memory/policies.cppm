export module gbox.core.memory:policies;

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

export namespace memory {

template <typename Self>
class MoveOnly {
  protected:
    MoveOnly() = default;
    ~MoveOnly() = default;
    MoveOnly(MoveOnly &&) noexcept = default;
    MoveOnly &operator=(MoveOnly &&) noexcept = default;
    MoveOnly(const MoveOnly &) = delete;
    MoveOnly &operator=(const MoveOnly &) = delete;
};

template <typename Self>
class Pinned {
  protected:
    Pinned() = default;
    ~Pinned() = default;
    Pinned(const Pinned &) = delete;
    Pinned &operator=(const Pinned &) = delete;
    Pinned(Pinned &&) = delete;
    Pinned &operator=(Pinned &&) = delete;
};

}  // namespace memory
