module;

#include <cassert>

#include "gbox/core/result.hpp"

export module gbox.core:result;

export namespace result {

using result::Err;
using result::Ok;
using result::Result;

using result::match;
using result::match_result;
using result::overloaded;

}  // namespace result
