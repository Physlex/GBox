module;

//! This module implements share-able type traits for the chain types

#include <concepts>
#include <type_traits>

export module gbox.funky.fn:chain.traits;

export namespace chain::traits {

template <class Self>
struct Chainable {};

template <class T, class U = std::remove_cvref_t<T>>
concept IsChainable = std::derived_from<U, Chainable<U>>;

}  // namespace chain::traits
