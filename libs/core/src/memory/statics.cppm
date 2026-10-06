module;

//! This module implements the `Static` type, a wrapper which holds a value that is never
//! destroyed, and the `StaticRef` type, a handle to a value which lives for the length of
//! the program.
//!
//! A value handed to a `Static` is constructed in place and abandoned: no destructor is
//! ever run against it, and the wrapper itself is trivially destructible, so an object of
//! static storage duration holding one registers nothing to run at exit.
//!
//! A `StaticRef` is formed from a `Static`, the only storage in the toolkit which never
//! destroys what it holds. Holding one is therefore a claim the compiler can carry
//! through a program: the value behind it cannot be destroyed, so a reference to it
//! cannot dangle.
//!
//! Prefer taking a `StaticRef<T>` over a `T &` wherever a type is expected to outlive its
//! caller, since the reference then carries that expectation itself.

#include <concepts>
#include <new>
#include <type_traits>
#include <utility>

export module gbox.core.memory:statics;

import gbox.core.mixins;
import :policies;

export namespace memory {

/// Holds a value of type `T` which outlives every scope it appears in.
///
/// # Example
/// ```cpp
/// auto counter = Static<Counter>(0);
/// counter.get().tick();
/// ```
template <typename T>
class Static final : public For<Static<T>, Where<MoveOnly<Self>>> {
  public:
    /// Constructs the value in place from the arguments given
    template <typename... Args>
        requires std::constructible_from<T, Args...>
    explicit constexpr Static(Args &&...args) : impl_(std::forward<Args>(args)...) {}

    /// Constructs a value from the one another `Static` holds
    ///
    /// The value left behind is abandoned rather than emptied, since nothing is ever
    /// destroyed out of a `Static`.
    Static(Static &&other) noexcept : impl_(std::move(other.get())) {}

    /// Overwrites the held value with the one another `Static` holds
    Static &operator=(Static &&other) noexcept {
        this->get() = std::move(other.get());
        return *this;
    }

    /// Borrows the held value mutably
    T &get() { return *this->impl_.get(); }

    /// Borrows the held value
    const T &get() const { return *this->impl_.get(); }

  private:
    /// Holds the value as a member, for a `T` which has no destructor to suppress.
    ///
    /// Construction stays constant-evaluable this way, so a `Static` over such a type can
    /// be initialized before the program runs rather than on first use.
    class DirectImpl {
      public:
        template <typename... Args>
        explicit constexpr DirectImpl(Args &&...args)
            : value_(std::forward<Args>(args)...) {}

        T *get() { return &this->value_; }
        const T *get() const { return &this->value_; }

      private:
        T value_;
    };

    /// Holds untyped storage the value is constructed into, for a `T` whose destructor
    /// must never run.
    ///
    /// The storage outlives the value it holds and carries no destructor of its own,
    /// which is what keeps `~T` unreachable.
    ///
    // The buffer is storage rather than a value, and the value is built into it directly.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    class PlacementImpl {
      public:
        template <typename... Args>
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        explicit PlacementImpl(Args &&...args) {
            ::new (static_cast<void *>(&this->space_)) T(std::forward<Args>(args)...);
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        T *get() { return std::launder(reinterpret_cast<T *>(&this->space_)); }

        const T *get() const {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            return std::launder(reinterpret_cast<const T *>(&this->space_));
        }

      private:
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
        alignas(T) unsigned char space_[sizeof(T)];
    };

    std::conditional_t<std::is_trivially_destructible_v<T>, DirectImpl, PlacementImpl>
        impl_;
};

/// Points at a value which is never destroyed.
///
/// # Example
/// ```cpp
/// auto counter = Static<Counter>(0);
/// auto ref = StaticRef<Counter>(counter);
/// ref->tick();
/// ```
template <typename T>
class StaticRef final : public For<StaticRef<T>, Where<MoveOnly<Self>>> {
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

}  // namespace memory
