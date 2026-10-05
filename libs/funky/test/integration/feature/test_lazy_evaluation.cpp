//! This file implements a feature test for `funky` demonstrating lazy evaluation: lazy
//! work captures its operands when it is built, can be handed to a consumer by impl or by
//! erased reference, and runs only once it is yielded.

#include <gtest/gtest.h>

#include <cstdint>
#include <utility>

#include "lib.hpp"

import gbox.funky;

using fn::lazily;
using fn::once;
using fn::Yield;
using fn::YieldImpl;

namespace {

constexpr int32_t LHS = 4;
constexpr int32_t RHS = 5;
constexpr int32_t CLOBBERED = 0;
constexpr int32_t EXPECTED_SUM = LHS + RHS;
constexpr int32_t EXPECTED_SHIFTED_SUM = shift(add(LHS, RHS));

constexpr int32_t EXPECTED_CALLS_DEFERRED = 0;
constexpr int32_t EXPECTED_CALLS_YIELDED = 1;
constexpr int32_t EXPECTED_CALLS_RAN_AT_CALL = 1;

/// Addition that counts how many times it has run, so a test can tell whether work was
/// deferred or done
constexpr auto counting_add(int32_t &calls) {
    return [&calls](int32_t a, int32_t b) -> int32_t {
        ++calls;
        return add(a, b);
    };
}

/// Shifting that counts how many times it has run, so a test can tell whether work was
/// deferred or done
constexpr auto counting_shift(int32_t &calls) {
    return [&calls](int32_t value) -> int32_t {
        ++calls;
        return shift(value);
    };
}

/// Runs deferred work it knows only by the type that work yields
constexpr int32_t consume_impl(YieldImpl<int32_t> auto work) {
    return std::move(work).yield();
}

/// Runs deferred work it knows only through the erased `Yield` view
constexpr int32_t consume_dyn(Yield<int32_t> &work) { return std::move(work).yield(); }

/// Builds lazy addition, handing back the deferred work rather than its result
constexpr YieldImpl<int32_t> auto make_add(int32_t a, int32_t b) {
    return lazily(once(add))(a, b);
}

}  // namespace

static_assert(
    YieldImpl<decltype(make_add(LHS, RHS)), int32_t>,
    "a lazy call hands back deferred work rather than a value"
);

static_assert(
    EXPECTED_SUM == consume_impl(make_add(LHS, RHS)),
    "lazy work consumed by impl yields its result during constant evaluation"
);

static_assert(
    EXPECTED_SUM ==
        [] {
            auto work = make_add(LHS, RHS);
            return consume_dyn(work);
        }(),
    "lazy work consumed by erased reference yields its result during constant evaluation"
);

static_assert(
    EXPECTED_SUM ==
        [] {
            int32_t a = LHS;
            int32_t b = RHS;
            auto work = lazily(once(add))(a, b);
            a = CLOBBERED;
            return std::move(work).yield();
        }(),
    "lazy work captures its operands when built, so changing them afterwards changes "
    "nothing"
);

static_assert(
    EXPECTED_SUM == consume_impl((lazily(add) << LHS)(RHS)),
    "lazy work completed through partial application yields its result during constant "
    "evaluation"
);

static_assert(
    EXPECTED_SHIFTED_SUM ==
        [] {
            auto work = (lazily(once(add)) | once(shift))(LHS, RHS);
            return std::move(work).yield();
        }(),
    "a chain beginning lazily yields the whole composition in one step"
);

static_assert(
    EXPECTED_SHIFTED_SUM ==
        [] {
            auto work = (once(add) | lazily(once(shift)))(LHS, RHS);
            return std::move(work).yield();
        }(),
    "a chain ending lazily yields its last stage in one step"
);

static_assert(
    EXPECTED_SHIFTED_SUM ==
        [] {
            auto work = (lazily(once(add)) | lazily(once(shift)))(LHS, RHS);
            auto shifted = std::move(work).yield();
            return std::move(shifted).yield();
        }(),
    "a chain of two lazy stages yields one step per lazy stage"
);

/// Lazy work passed by impl runs only once its consumer yields it
TEST(lazyImplParameter, lazyEvaluationTests) {
    int32_t calls = 0;
    YieldImpl<int32_t> auto work = lazily(once(counting_add(calls)))(LHS, RHS);

    ASSERT_EQ(EXPECTED_CALLS_DEFERRED, calls) << "work ran before it was handed off";
    ASSERT_EQ(EXPECTED_SUM, consume_impl(std::move(work)))
        << "work yielded a wrong result";
    ASSERT_EQ(EXPECTED_CALLS_YIELDED, calls) << "work did not run exactly once on yield";
}

/// Lazy work passed by erased reference runs only once its consumer yields it
TEST(lazyDynParameter, lazyEvaluationTests) {
    int32_t calls = 0;
    YieldImpl<int32_t> auto work = lazily(once(counting_add(calls)))(LHS, RHS);

    ASSERT_EQ(EXPECTED_CALLS_DEFERRED, calls) << "work ran before it was handed off";
    ASSERT_EQ(EXPECTED_SUM, consume_dyn(work)) << "work yielded a wrong result";
    ASSERT_EQ(EXPECTED_CALLS_YIELDED, calls) << "work did not run exactly once on yield";
}

/// Addition over values only known at runtime is deferred until yielded
TEST(lazyRuntimeAddition, lazyEvaluationTests) {
    int32_t calls = 0;
    int32_t a = LHS;
    int32_t b = RHS;
    YieldImpl<int32_t> auto work = lazily(once(counting_add(calls)))(a, b);
    a = CLOBBERED;

    ASSERT_EQ(EXPECTED_CALLS_DEFERRED, calls) << "work ran before it was yielded";
    ASSERT_EQ(EXPECTED_SUM, std::move(work).yield())
        << "work did not keep its bound operands";
    ASSERT_EQ(EXPECTED_CALLS_YIELDED, calls) << "work did not run exactly once on yield";
}

/// Lazy work completed through partial application runs only once it is yielded
TEST(lazyPartialApplication, lazyEvaluationTests) {
    int32_t calls = 0;
    YieldImpl<int32_t> auto work = (lazily(once(counting_add(calls))) << LHS)(RHS);

    ASSERT_EQ(EXPECTED_CALLS_DEFERRED, calls) << "work ran before it was yielded";
    ASSERT_EQ(EXPECTED_SUM, std::move(work).yield()) << "work yielded a wrong result";
    ASSERT_EQ(EXPECTED_CALLS_YIELDED, calls) << "work did not run exactly once on yield";
}

/// A chain beginning lazily runs none of its stages until it is yielded
TEST(lazyChainFirstStage, lazyEvaluationTests) {
    int32_t add_calls = 0;
    int32_t shift_calls = 0;
    YieldImpl<int32_t> auto work =
        (lazily(once(counting_add(add_calls))) |
         once(counting_shift(shift_calls)))(LHS, RHS);

    ASSERT_EQ(EXPECTED_CALLS_DEFERRED, add_calls) << "first stage ran before the yield";
    ASSERT_EQ(EXPECTED_CALLS_DEFERRED, shift_calls)
        << "second stage ran before the yield";
    ASSERT_EQ(EXPECTED_SHIFTED_SUM, std::move(work).yield())
        << "chain yielded a wrong result";
    ASSERT_EQ(EXPECTED_CALLS_YIELDED, add_calls)
        << "first stage did not run once on yield";
    ASSERT_EQ(EXPECTED_CALLS_YIELDED, shift_calls)
        << "second stage did not run once on yield";
}

/// A chain ending lazily runs its eager stage at the call and defers only its last
TEST(lazyChainSecondStage, lazyEvaluationTests) {
    int32_t add_calls = 0;
    int32_t shift_calls = 0;
    YieldImpl<int32_t> auto work =
        (once(counting_add(add_calls)) |
         lazily(once(counting_shift(shift_calls))))(LHS, RHS);

    ASSERT_EQ(EXPECTED_CALLS_RAN_AT_CALL, add_calls)
        << "eager stage did not run at the call";
    ASSERT_EQ(EXPECTED_CALLS_DEFERRED, shift_calls) << "lazy stage ran before the yield";
    ASSERT_EQ(EXPECTED_SHIFTED_SUM, std::move(work).yield())
        << "chain yielded a wrong result";
    ASSERT_EQ(EXPECTED_CALLS_YIELDED, shift_calls)
        << "lazy stage did not run once on yield";
}

/// A chain of two lazy stages defers each stage behind its own yield
TEST(lazyChainBothStages, lazyEvaluationTests) {
    int32_t add_calls = 0;
    int32_t shift_calls = 0;
    auto work =
        (lazily(once(counting_add(add_calls))) |
         lazily(once(counting_shift(shift_calls))))(LHS, RHS);

    ASSERT_EQ(EXPECTED_CALLS_DEFERRED, add_calls) << "first stage ran before any yield";
    ASSERT_EQ(EXPECTED_CALLS_DEFERRED, shift_calls)
        << "second stage ran before any yield";

    YieldImpl<int32_t> auto shifted = std::move(work).yield();

    ASSERT_EQ(EXPECTED_CALLS_YIELDED, add_calls)
        << "first stage did not run on the first yield";
    ASSERT_EQ(EXPECTED_CALLS_DEFERRED, shift_calls)
        << "second stage ran on the first yield";
    ASSERT_EQ(EXPECTED_SHIFTED_SUM, std::move(shifted).yield())
        << "chain yielded a wrong result";
    ASSERT_EQ(EXPECTED_CALLS_YIELDED, shift_calls)
        << "second stage did not run on its yield";
}
