export module gbox.func_ky:func;

import :func.once;
import :func.partial;
import :func.thunk;

export namespace func {

using once::make_once;
using once::OnceFn;
using partial::Partial;
using thunk::make_thunk;
using thunk::Thunk;

}  // namespace func
