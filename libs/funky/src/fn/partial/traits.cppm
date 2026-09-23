module;

//! This module implements share-able type traits for the partial application types

#include <concepts>
#include <type_traits>

export module gbox.funky.fn:partial.traits;

export namespace partial::traits {

template <class Self>
struct Bindable {};

template <class T, class U = std::remove_cvref_t<T>>
concept IsBindable = std::derived_from<U, Bindable<U>>;

}  // namespace partial::traits
