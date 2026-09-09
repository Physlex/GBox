module;

//! This module implements the `StaticRef` type, a handle to a value which lives for the
//! length of the program.
//!
//! A `StaticRef` is formed from a `Static`, the only storage in the toolkit which never
//! destroys what it holds. Holding one is therefore a claim the compiler can carry
//! through a program: the value behind it cannot be destroyed, so a reference to it
//! cannot dangle.
//!
//! Prefer taking a `StaticRef<T>` over a `T &` wherever a type is expected to outlive its
//! caller, since the reference then carries that expectation itself.

export module gbox.cell:static_ref;

import :static_t;

import gbox.core;

using memory::MoveOnly;
using static_t::Static;

export namespace static_ref {

/// Points at a value which is never destroyed.
///
/// # Example
/// ```cpp
/// auto counter = Static<Counter>(0);
/// auto ref = StaticRef<Counter>(counter);
/// ref->tick();
/// ```
template <typename T>
class StaticRef final : public MoveOnly {
  public:
    /// Points the reference at the value a `Static` holds
    explicit StaticRef(Static<T> &leaked) : ptr_(&leaked.get()) {}

    /// Borrows the referenced value
    T &operator*() const { return *this->ptr_; }

    /// Reaches a member of the referenced value
    T *operator->() const { return this->ptr_; }

    /// Borrows the referenced value
    T &get() const { return *this->ptr_; }

  private:
    T *ptr_;
};

}  // namespace static_ref
