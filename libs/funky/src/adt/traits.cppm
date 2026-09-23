module;

//! This module defines the type traits shared by the library's variant-backed sum types.

#include <functional>
#include <type_traits>

export module gbox.funky.adt:traits;

export namespace traits {

/// Names what a variant is able to hold as one of its alternatives
///
/// Void and array types have no alternative a variant can give them. A reference passes,
/// on the understanding that whatever holds it keeps it by wrapper rather than placing it
/// in the variant directly.
template <typename T>
concept UnionStorable = std::is_object_v<std::remove_reference_t<T>> &&
                        !std::is_array_v<std::remove_reference_t<T>>;

/// The type a variant-backed holder places in its storage
///
/// A variant holds no reference of its own, so a reference is kept by wrapper instead.
/// The wrapper stores the address of whatever it was bound to, leaving the referent where
/// it is.
template <typename T>
using Stored = std::conditional_t<
    std::is_reference_v<T>, std::reference_wrapper<std::remove_reference_t<T>>, T>;

}  // namespace traits
