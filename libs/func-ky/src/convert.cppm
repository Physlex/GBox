module;

//! This module defines the `Into` trait, which establishes strict connections between
//! types

export module gbox.func_ky:convert;

import :result;
import :func;

using func::FnOnce;

namespace convert {

/// This policy defines conversion of types into some base type implementing this
/// trait to some conversion type `T`
template <class Derived, typename T>
struct Into {
    /// Converts a value from Derived into T
    T into(Derived &&value);
};

/// This policy defines conversion of types from some foreign type implementing this trait
/// to the derived
template <class Derived, typename T>
struct From {
    /// Converts a value into Derived from T
    Derived from(T &&value);
};

template <class Derived, typename Fn>
struct Map {
    /// Converts a Derived type into a type specified by T
    auto map(FnOnce<Fn> &&f);
};

}  // namespace convert
