#ifndef GBOX_CORE_RESULT_HPP_
#define GBOX_CORE_RESULT_HPP_

//! This module implements result-style error propagation semantics for C++.

#include <utility>
#include <variant>

namespace gbox::result {

/// Alias for the result storage type
template <typename T, typename E>
using ResultInner =
    std::variant<std::conditional_t<std::is_void_v<T>, std::monostate, T>, E>;

/// Forward declaration of Ok with a default void type parameter, enabling
/// `Ok()` to be written without explicit template arguments when representing
/// a successful result with no value.
///
/// # Example
/// ```cpp
/// Result<void, int> res = Ok();
/// ```
template <typename T = void>
struct Ok;

/// Deduction guide for the Ok result builder
template <typename T>
Ok(T) -> Ok<T>;

/// Proxy-class for a Result which has a valid value
template <typename T>
struct Ok {
    T value;
    Ok(T v) : value(std::move(v)) {}
};

/// Specialization for void-case Ok types
template <>
struct Ok<void> {
    Ok() = default;
};

/// Proxy-class for a Result which has an erroneous value
template <typename E>
struct Err {
    E value;
    Err(E v) : value(std::move(v)) {}
};

/// Deduction guide for the Err result builder
template <typename E>
Err(E) -> Err<E>;

/// Pseudo-functional result type, for error propagation without needing to use the cpp
/// expected type
template <typename T, typename E>
class Result {
  public:
    /// Constructs a Result from an Ok value
    Result(Ok<T> &&o) {
        if constexpr (std::is_void_v<T>) {
            this->inner_ = std::monostate();
        } else {
            this->inner_ = std::move(o.value);
        }
    };

    /// Constructs a Result from an erroneous value
    Result(Err<E> &&e) : inner_(std::move(e.value)) {};

    /// Returns true if the result holds an erroneous value
    inline bool is_err() { return std::holds_alternative<E>(this->inner_); }

    /// Returns true if the result holds a valid value
    inline bool is_ok() {
        if constexpr (std::is_void_v<T>) {
            return std::holds_alternative<std::monostate>(this->inner_);
        } else {
            return std::holds_alternative<T>(this->inner_);
        }
    }

    /// Returns the valid value. Throws `std::bad_variant_access` if the result is
    /// erroneous
    inline T unwrap() {
        if constexpr (std::is_void_v<T>) {
            std::get<std::monostate>(this->inner_);
            return;
        } else {
            return std::get<T>(this->inner_);
        }
    }

    /// Returns the erroneous value. Throws `std::bad_variant_access` if the result is
    /// valid
    inline E unwrap_err() { return std::get<E>(this->inner_); }

    inline ResultInner<T, E> inner() const { return this->inner_; }

  private:
    ResultInner<T, E> inner_;
};

// FIXME: Remove the dispatch-based result type once we get the opportunity to make a
// better ADT

/// Standard "overloaded" type definition. Forces the "visit" method to use all possible
/// variants...
///
/// Effectively creates an anonymous struct of match arms (anything that defines the ()
/// operator, really)
///
/// When used in conjunction with "visit", the compiler will require every possible option
/// to exist for the struct to be considered "good".
template <class... Match>
struct overloaded : Match... {
    using Match::operator()...;
};

/// This function attempts to make generic type handling something more bearable
///
/// Does some template magic to pack a set of match arm callbacks (which are type defined
/// as a list of lambda callbacks using the overload struct magic above) into a single
/// "forwarded" lambda.
///
/// Simpler terms, it allows matching against an std::variant and forces the user to
/// define a match arm for every possible variant that the sum type defines.
template <typename AbstractSumType, typename... Matches>
auto match(AbstractSumType &&variant, Matches &&...matches) {
    return std::visit(
        overloaded{std::forward<Matches>(matches)...},
        std::forward<AbstractSumType>(variant)
    );
}

/// Experimental. Match specialization for a result type, which forces an "Err" and "Ok"
/// match arm. Does not support `Result<void, E>`.
template <typename T, typename E, typename OkMatch, typename ErrMatch>
auto match_result(Result<T, E> &&res, OkMatch &&ok_match, ErrMatch &&err_match) {
    return match(
        std::move(res.inner()), [&](T &&val) { return ok_match(Ok<T>{std::move(val)}); },
        [&](E &&err) { return err_match(Err<E>{std::move(err)}); }
    );
}

}  // namespace gbox::result

#endif  // GBOX_CORE_RESULT_HPP_
