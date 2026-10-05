export module gbox.funky.fn;

export import :chain;
export import :chain.traits;
export import :lazy;
export import :lazy.traits;
export import :once;
export import :partial;
export import :partial.traits;
export import :yield;
export import :yield.thunk;
export import :yield.traits;

export namespace fn {

using chain::Chain;
using chain::traits::Chainable;
using lazy::lazily;
using lazy::Lazy;
using once_fn::once;
using once_fn::OnceFn;
using partial::Partial;
using partial::traits::Bindable;
using yield::Yield;
using yield::thunk::Thunk;
using yield::traits::YieldImpl;

}  // namespace fn
