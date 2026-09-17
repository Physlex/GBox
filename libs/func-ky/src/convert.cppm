module;

//! This module defines the `Into` trait, which establishes strict connections between
//! types

export module gbox.func_ky:convert;

import :result;
import :func;

using func::OnceFn;

namespace convert {

/// This policy defines conversion of types into some base type implementing this
/// trait to some conversion type `T`
export template <class Self, typename T>
struct Into {
    /// Converts a value from Self into T
    T into(Self &&value);
};

}  // namespace convert
