export module gbox.funky.fn;

export import :chain;
export import :chain.traits;
export import :lazy;
export import :lazy.traits;
export import :once;
export import :partial;
export import :partial.traits;
export import :thunk;

export namespace fn {

using chain::Chain;
using chain::traits::Chainable;
using lazy::Lazy;
using lazy::lazy;
using once_fn::once;
using once_fn::OnceFn;
using partial::Partial;
using partial::traits::Bindable;
using thunk::Thunk;

}  // namespace fn
