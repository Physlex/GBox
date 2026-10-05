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

/// Whether a type is itself a composition of two stages
template <class T>
inline constexpr bool is_chain = false;

/// Concept form of [`is_chain`]
template <class T>
concept IsChain = is_chain<std::remove_cvref_t<T>>;

}  // namespace chain::traits
