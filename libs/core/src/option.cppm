module;

#include <cassert>
#include <variant>

export module gbox.core:option;

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
    Option(Some<T> &&some) { this->inner_ = std::move(some.value); }
    Option(None none) { this->inner_ = None(); }

    /// Transform the option type from type Option<T> to type Option<U>, where U is the
    /// return type of the entered lambda Fn
    ///
    /// Maps the options inner type to a type specified by the lambda. Expects that the
    /// option is `Some`. If it is instead `None`, then return early as the `None` type.
    template <typename Fn>
    [[nodiscard]]
    auto map(Fn &&f) -> Option<decltype(f(std::declval<T>()))> {
        if (this->is_none()) {
            return None();
        }

        return Some(f(std::get<T>(this->inner_)));
    }

    /// Assume that the some type is as specified, and if not, throw an exception
    [[nodiscard]]
    inline T &assume_some() const {
        assert(std::holds_alternative<T>(this->inner_));
        return std::get<T>(this->inner_);
    }

    /// Checks if the inner value is a some type
    inline bool is_some() const { return std::holds_alternative<T>(this->inner_); }

    /// Checks if the inner value is a none type
    inline bool is_none() const { return std::holds_alternative<None>(this->inner_); }

  private:
    OptionInner<T> inner_;
};

}  // namespace option
