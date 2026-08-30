module;

#include <exception>
#include <variant>

export module gbox.func_ky:option;

//! This module implements option typing to promote null-and-missing errors to the
//! compiler

export namespace option {

/// Empty type representing a lack of a value
struct None {};

/// Alias for the Option storage type
template <typename T>
using OptionInner = std::variant<T, None>;

template <typename T>
struct Some;

template <typename T>
Some(T) -> Some<T>;

template <typename T>
struct Some {
    T value;
    explicit Some(T v) : value(std::move(v)) {}
};

template <typename T>
class Option {
  public:
    Option() = delete;
    Option(Some<T> &&some) : inner_(std::move(some).value) {}
    Option(None none) : inner_(None()) {}

    /// Transform the option type from type Option<T> to type Option<U>, where U is the
    /// return type of the entered lambda Fn
    ///
    /// Maps the options inner type to a type specified by the lambda. Expects that the
    /// option is `Some`. If it is instead `None`, then return early as the `None` type.
    template <typename Fn>
    auto map(Fn &&f) -> Option<decltype(f(std::declval<T>()))> {
        [[unlikely]]
        if (this->is_none()) {
            return None();
        }

        return Some(std::forward<Fn>(f)(std::get<T>(this->inner_)));
    }

    /// Take the contained value out of the option type, replacing it with a `None` type,
    /// instead.
    ///
    /// Map of Option<Some(T)> -> Option<None>, returning the inner T value.
    ///
    /// ## Error
    /// Assumes the inner type is `Some`, if not, aborts.
    T &&take() {
        [[unlikely]]
        if (!this->is_some()) {
            std::terminate();
        }

        auto res = std::move(this->inner_);
        this->inner_ = None();
        return res;
    }

    /// Assume that the option type holds a valid type, then copies it for the consumer
    ///
    /// ## Error
    /// If the option type is actually a `None` type, then the program will abort.
    T assume_some() const noexcept {
        [[unlikely]]
        if (this->is_none()) {
            std::terminate();
        }

        return this->inner_;
    }

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
    [[nodiscard]] bool is_some() const { return std::holds_alternative<T>(this->inner_); }

    /// Checks if the inner value is a none type
    [[nodiscard]] bool is_none() const {
        return std::holds_alternative<None>(this->inner_);
    }

  private:
    OptionInner<T> inner_;
};

}  // namespace option
