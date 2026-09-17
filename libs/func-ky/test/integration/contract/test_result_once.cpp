//! This file implements testing for the contract between the result type and the owned
//! callables it maps through

#include <gtest/gtest.h>

#include <memory>
#include <utility>

import gbox.func_ky;

using func::OnceFn;
using result::Err;
using result::Ok;

enum class ErrorKind : uint8_t { Any };

template <typename T>
using Result = result::Result<T, ErrorKind>;

namespace {

/// A value that cannot be copied, so a map which copies where it should have moved fails
/// to build rather than passing quietly
using Owned = std::unique_ptr<int32_t>;

/// The value a test puts into a result, so that reading one back names which it was
constexpr int32_t HELD = 42;

}  // namespace

/// Map a spent result and verify the value it held was transformed
TEST(resultMapSpends, resultOnceTests) {
    Result<int32_t> res = Ok(HELD);

    Result<int32_t> mapped =
        std::move(res).map(OnceFn([](int32_t held) { return held + 1; }));

    ASSERT_TRUE(mapped.is_ok());
    ASSERT_EQ(HELD + 1, std::move(mapped).assume_ok());
}

/// Verify a map changes the type the result holds on its valid side
TEST(resultMapChangesType, resultOnceTests) {
    Result<int32_t> res = Ok(HELD);

    Result<bool> mapped =
        std::move(res).map(OnceFn([](int32_t held) { return held == HELD; }));

    ASSERT_TRUE(std::move(mapped).assume_ok());
}

/// Verify mapping an erroneous result leaves the callable uninvoked and keeps the error
TEST(resultMapErrSkipsCallable, resultOnceTests) {
    Result<int32_t> res = Err(ErrorKind::Any);
    bool invoked = false;

    Result<int32_t> mapped = std::move(res).map(OnceFn([&invoked](int32_t held) {
        invoked = true;
        return held;
    }));

    ASSERT_TRUE(mapped.is_err());
    ASSERT_FALSE(invoked);
    ASSERT_EQ(ErrorKind::Any, std::move(mapped).assume_err());
}

/// Verify a map hands a move-only value to the callable rather than copying it
TEST(resultMapMoveOnly, resultOnceTests) {
    Result<Owned> res = Ok(std::make_unique<int32_t>(HELD));

    Result<int32_t> mapped = std::move(res).map(OnceFn([](Owned held) { return *held; }));

    ASSERT_EQ(HELD, std::move(mapped).assume_ok());
}

/// Verify a map carries a move-only error across without copying it
TEST(resultMapMoveOnlyErr, resultOnceTests) {
    result::Result<int32_t, Owned> res = Err(std::make_unique<int32_t>(HELD));

    result::Result<int32_t, Owned> mapped =
        std::move(res).map(OnceFn([](int32_t held) { return held + 1; }));

    ASSERT_TRUE(mapped.is_err());
    ASSERT_EQ(HELD, *std::move(mapped).assume_err());
}
