module;

//! This module implements option typing to promote null-and-missing errors to the
//! compiler

#include <exception>
#include <type_traits>
#include <utility>
#include <variant>

export module gbox.func_ky:option;

import :func.once;
import :traits;

using once_fn::OnceFn;
using traits::Stored;
using traits::UnionStorable;

namespace option {

/// Empty type representing a lack of a value
export struct None {};

/// Alias for the Option storage type
template <typename T>
using OptionInner = std::variant<Stored<T>, None>;

/// Names what an option is able to hold
///
/// Narrows what a variant accepts by the one case an option reserves for itself: `None`
/// is the emptiness, never something the filled case carries.
template <typename T>
concept OptionStorable =
    UnionStorable<T> && !std::is_same_v<std::remove_cvref_t<T>, None>;

export template <typename T>
struct Some;

export template <typename T>
Some(T) -> Some<T>;

export template <typename T>
struct Some {
    T value;

    /// Adopts the value the option will hold
    explicit Some(T v) : value(std::forward<T>(v)) {}
};

export template <OptionStorable T>
class Option {
  public:
    Option() = delete;

    /// Adopts the value a `Some` carries, spending the `Some` in the process
    Option(Some<T> &&some) : inner_(std::move(some).value) {}

    /// Constructs an option holding nothing
    Option(None none) : inner_(None()) {}

    /// Transform the option type from type Option<T> to type Option<U>, where U is the
    /// return type of the owned callable
    ///
    /// Spends the option and the callable together, handing the contained value over to
    /// be transformed. If the option is a `None`, then return early as the `None` type,
    /// leaving the callable uninvoked.
    template <typename F>
    auto map(OnceFn<F> f) && -> Option<std::invoke_result_t<OnceFn<F>, T>> {
        using Mapped = std::invoke_result_t<OnceFn<F>, T>;

        [[unlikely]]
        if (this->is_none()) {
            return None();
        }

        return Some<Mapped>(std::move(f)(std::forward<T>(this->held())));
    }

    /// Transform the option type from type Option<T> to type Option<U>, leaving the
    /// option it was called on holding what it already held
    ///
    /// Lends the contained value to the callable rather than surrendering it. If the
    /// option is a `None`, then return early as the `None` type, leaving the callable
    /// uninvoked.
    template <typename F>
    auto map(OnceFn<F> f) const & -> Option<std::invoke_result_t<OnceFn<F>, const T &>> {
        using Mapped = std::invoke_result_t<OnceFn<F>, const T &>;

        [[unlikely]]
        if (this->is_none()) {
            return None();
        }

        return Some<Mapped>(std::move(f)(this->held()));
    }

    /// Take the contained value out of the option type, replacing it with a `None` type,
    /// instead.
    ///
    /// Map of Option<Some(T)> -> Option<None>, returning the value taken out as an
    /// option of its own. A `None` returns a `None`.
    Option<T> take() noexcept(std::is_nothrow_move_constructible_v<T>) {
        [[unlikely]]
        if (this->is_none()) {
            return None();
        }

        Option<T> taken = Some<T>(std::forward<T>(this->held()));
        this->inner_ = None();

        return taken;
    }

    /// Assume that the option type holds a valid type, then moves it out for the consumer
    ///
    /// Spends the option it is called on, since what stays behind has been moved from.
    ///
    /// ## Error
    /// If the option type is actually a `None` type, then the program will abort.
    T assume_some() && noexcept(std::is_nothrow_move_constructible_v<T>) {
        [[unlikely]]
        if (this->is_none()) {
            std::terminate();
        }

        return std::forward<T>(this->held());
    }

    /// Borrows the contained value mutably, leaving it in the option
    ///
    /// Use this over `assume_some` whenever the address of the contained value matters,
    /// such as when a reference to it outlives the call.
    ///
    /// ## Error
    /// If the option type is actually a `None` type, then the program will abort.
    T &as_mut() noexcept { return this->held(); }

    /// Borrows the contained value, leaving it in the option
    ///
    /// An option that cannot be written to still lends out a mutable referent when `T`
    /// is a reference, since the binding is the option's and the value it names is not.
    ///
    /// ## Error
    /// If the option type is actually a `None` type, then the program will abort.
    const T &as_ref() const noexcept { return this->held(); }

    /// Assume that the option type holds nothing
    ///
    /// ## Error
    /// If the option type is actually a `Some` type, then the program will abort.
    void assume_none() const noexcept {
        [[unlikely]]
        if (this->is_some()) {
            std::terminate();
        }
    }

    /// Checks if the inner value is a some type
    [[nodiscard]] bool is_some() const noexcept {
        return std::holds_alternative<Stored<T>>(this->inner_);
    }

    /// Checks if the inner value is a none type
    [[nodiscard]] bool is_none() const noexcept {
        return std::holds_alternative<None>(this->inner_);
    }

  private:
    /// Reaches the held value, unwrapping the wrapper when `T` is a reference
    ///
    /// Every accessor reads through here, so the reference and value cases part company
    /// in one place rather than in each of them. A const option holding a reference
    /// still yields a mutable referent: the binding belongs to the option, the value it
    /// names does not.
    ///
    /// ## Error
    /// If the option holds nothing, then the program will abort.
    template <typename Self>
    decltype(auto) held(this Self &&self) noexcept {
        auto *stored = std::get_if<Stored<T>>(&std::forward<Self>(self).inner_);

        [[unlikely]]
        if (stored == nullptr) {
            std::terminate();
        }

        if constexpr (std::is_reference_v<T>) {
            return stored->get();
        } else {
            return *stored;
        }
    }

    OptionInner<T> inner_;
};

}  // namespace option
