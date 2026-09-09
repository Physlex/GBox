module;

//! This module implements result-style error propagation semantics for C++.

#include <cstddef>
#include <exception>
#include <type_traits>
#include <utility>
#include <variant>

export module gbox.func_ky:result;

import :func.once;
import :traits;

using once::OnceFn;
using traits::UnionStorable;

namespace result {

/// Alias for the result storage type
template <typename T, typename E>
using ResultInner =
    std::variant<std::conditional_t<std::is_void_v<T>, std::monostate, T>, E>;

/// Names what a result is able to hold
///
/// A result carrying nothing on success spells its valid side as a monostate, so a void
/// `T` passes where an option would refuse one. The erroneous side always names a value.
///
/// Neither side takes a reference. A variant holds none directly, and unlike an option a
/// result places what it is given into storage unwrapped.
template <typename T, typename E>
concept ResultStorable =
    (std::is_void_v<T> || (UnionStorable<T> && !std::is_reference_v<T>)) &&
    (UnionStorable<E> && !std::is_reference_v<E>);

/// Forward declaration of Ok with a default void type parameter, enabling
/// `Ok()` to be written without explicit template arguments when representing
/// a successful result with no value.
///
/// # Example
/// ```cpp
/// Result<void, int> res = Ok();
/// ```
export template <typename T = void>
struct Ok;

/// Deduction guide for the Ok result builder
export template <typename T>
Ok(T) -> Ok<T>;

/// Proxy-class for a Result which has a valid value
export template <typename T>
struct Ok {
    T value;

    /// Adopts the value the result will hold
    explicit Ok(T v) : value(std::forward<T>(v)) {}
};

/// Specialization for void-case Ok types
export template <>
struct Ok<void> {
    Ok() = default;
};

/// Proxy-class for a Result which has an erroneous value
export template <typename E>
struct Err {
    E value;

    /// Adopts the value the result will hold
    explicit Err(E v) : value(std::forward<E>(v)) {}
};

/// Deduction guide for the Err result builder
export template <typename E>
Err(E) -> Err<E>;

/// Pseudo-functional result type, for error propagation without needing to use the cpp
/// expected type
export template <typename T, typename E>
    requires ResultStorable<T, E>
class Result {
  public:
    /// Constructs a Result from an Ok value
    Result(Ok<T> &&o)
        requires(!std::is_void_v<T>)
        : inner_(std::in_place_index<OK_INDEX>, std::move(o).value) {};

    /// Constructs a Result from an Ok value carrying nothing
    Result(const Ok<T> &_o)
        requires(std::is_void_v<T>)
        : inner_(std::in_place_index<OK_INDEX>) {};

    /// Constructs a Result from an erroneous value
    Result(Err<E> &&e) : inner_(std::in_place_index<ERR_INDEX>, std::move(e).value) {};

    /// Transform the result type from type Result<T, E> to type Result<U, E>, where U is
    /// the return type of the owned callable
    ///
    /// Spends the result and the callable together, handing the valid value over to be
    /// transformed. If the result is erroneous, then return early as the `Err` type,
    /// leaving the callable uninvoked. A result carrying nothing on success has no value
    /// to transform, and so has no map at all.
    template <typename F>
        requires(!std::is_void_v<T>)
    auto map(OnceFn<F> f) && -> Result<std::invoke_result_t<OnceFn<F>, T>, E> {
        using Mapped = std::invoke_result_t<OnceFn<F>, T>;

        [[unlikely]]
        if (this->is_err()) {
            return Err<E>(std::forward<E>(std::get<ERR_INDEX>(this->inner_)));
        }

        return Ok<Mapped>(
            std::move(f)(std::forward<T>(std::get<OK_INDEX>(this->inner_)))
        );
    }

    /// Returns the valid value, spending the result it is called on
    ///
    /// ## Error
    /// If the result is erroneous, then the program will abort.
    [[nodiscard]]
    T assume_ok() && noexcept(
        std::is_void_v<T> || std::is_nothrow_move_constructible_v<T>
    ) {
        [[unlikely]]
        if (this->is_err()) {
            std::terminate();
        }

        if constexpr (std::is_void_v<T>) {
            return;
        } else {
            return std::forward<T>(std::get<OK_INDEX>(this->inner_));
        }
    }

    /// Returns the erroneous value, spending the result it is called on
    ///
    /// ## Error
    /// If the result is valid, then the program will abort.
    [[nodiscard]]
    E assume_err() && noexcept(std::is_nothrow_move_constructible_v<E>) {
        [[unlikely]]
        if (this->is_ok()) {
            std::terminate();
        }

        return std::forward<E>(std::get<ERR_INDEX>(this->inner_));
    }

    /// Returns true if the result holds an erroneous value
    [[nodiscard]] bool is_err() const noexcept {
        return this->inner_.index() == ERR_INDEX;
    }

    /// Returns true if the result holds a valid value
    [[nodiscard]] bool is_ok() const noexcept { return this->inner_.index() == OK_INDEX; }

  private:
    /// The alternative the storage keeps a valid value in
    static constexpr std::size_t OK_INDEX = 0;

    /// The alternative the storage keeps an erroneous value in
    static constexpr std::size_t ERR_INDEX = 1;

    ResultInner<T, E> inner_;
};

}  // namespace result
