//! This module aggregates all of the public-facing api for gbox core

// IWYU pragma: begin_exports

namespace gbox::core {
#include "gbox/core/cell.hpp"
#include "gbox/core/memory.hpp"
#include "gbox/core/option.hpp"
#include "gbox/core/result.hpp"
#include "gbox/core/ring.hpp"
}  // namespace gbox::core

// NOTE: Not really something worth specifying in the namespaced context
#include "gbox/core/types.hpp"

// IWYU pragma: end_exports
