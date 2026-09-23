//! This file implements unit testing for the `Partial` type: how arguments were supplied
//! decides the shape of the layer that results, and both shapes cost the same storage.
//!
//! Every case here is settled while the file is compiled, so building this test is what
//! passes it.

#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>

import gbox.funky;

using fn::Partial;

namespace {

/// A callable carrying no state, so a layer over it owns nothing but its bound arguments
struct Stateless {
    constexpr auto operator()(int32_t a, int32_t b) const -> int32_t { return a + b; }
};

/// The same arity carrying state, so a layer over it owns that state as well
struct Stateful {
    int32_t base;

    constexpr auto operator()(int32_t a, int32_t b) const -> int32_t {
        return this->base + a + b;
    }
};

/// A layer holding nothing yet, which is what further arguments are supplied to
constexpr auto empty_layer() { return Partial<Stateless>(Stateless{}); }

/// A layer holding the first of the callable's two arguments
constexpr auto first_bound() { return Partial<Stateless, int32_t>(Stateless{}, 1); }

}  // namespace

static_assert(std::is_empty_v<Stateless>);
static_assert(!std::is_empty_v<Stateful>);

/// Nesting records how the arguments arrived, not what the layer stores, so every
/// arrangement of the same arguments over the same callable is one size.
static_assert(
    sizeof(Partial<Stateless, int32_t, int32_t>) ==
        sizeof(Partial<Partial<Stateless, int32_t>, int32_t>),
    "a flat layer over a stateless callable is the size of the nested layers it replaces"
);
static_assert(
    sizeof(Partial<Stateful, int32_t, int32_t>) ==
        sizeof(Partial<Partial<Stateful, int32_t>, int32_t>),
    "a flat layer over a stateful callable is the size of the nested layers it replaces"
);
static_assert(
    sizeof(Partial<Partial<Stateless, int32_t, int32_t>, int32_t>) ==
        sizeof(Partial<Partial<Stateless, int32_t>, int32_t, int32_t>),
    "where the arguments were split between two layers does not change their size"
);

/// A layer adds nothing of its own, so a stateless callable leaves it the size of the
/// arguments it holds.
static_assert(
    sizeof(Partial<Stateless, int32_t, int32_t>) <= sizeof(std::tuple<int32_t, int32_t>),
    "a layer over a stateless callable is at most the size of its bound arguments"
);

/// How the arguments were supplied is what decides which of the two shapes results
static_assert(
    std::same_as<
        decltype(first_bound() << 2), Partial<Partial<Stateless, int32_t>, int32_t> >,
    "binding one argument at a time nests one layer per argument"
);
static_assert(
    std::same_as<
        decltype(empty_layer() << std::tuple{1, 2}),
        Partial<Partial<Stateless>, int32_t, int32_t> >,
    "binding a tuple spreads its elements across one flat layer"
);

/// Either shape invokes the callable with the same arguments in the same order
static_assert(
    (first_bound() << 2)() == 3,
    "nested layers apply their arguments in the order they were bound"
);
static_assert(
    (empty_layer() << std::tuple{1, 2})() == 3,
    "a flat layer applies its arguments in the order the tuple held them"
);

/// Arguments supplied at the call are appended behind everything already bound
static_assert(
    Partial<Stateless, int32_t>(Stateless{}, 1)(2) == 3,
    "an argument given at the call arrives after the bound arguments"
);
