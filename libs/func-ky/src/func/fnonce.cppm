module;

//! This module defines the `FnOnce` object, a move-only wrapper around a lambda which
//! allows for simple ownership of function types.
//!
//! Importantly, it's heapless, meaning it's faster and safer by default.

#include <functional>

export module gbox.func_ky:func.fnonce;

import gbox.core;

using memory::MoveOnly;

export namespace fnonce {

// TODO: DOCS(Usage, Brief, Description)
template <typename Fn>
class FnOnce : public MoveOnly {
  public:
    FnOnce(Fn &&lambda) : lambda_(std::move(lambda)) {}

    /// Operator overload for the FnOnce type, consuming the type to invoke the inner
    /// lambda
    ///
    /// ## Returns
    /// Uses decltype(auto) pattern to "forward" the return type of the evaluated lambda
    /// directly to the owning type.
    template <typename... Args>
        requires std::invocable<Fn, Args...>
    decltype(auto) operator()(Args &&...args) && noexcept {
        return std::invoke(std::move(this->lambda_), std::forward<Args>(args)...);
    }

  private:
    Fn lambda_;
};

template <typename Fn>
FnOnce(Fn) -> FnOnce<Fn>;

}  // namespace fnonce
