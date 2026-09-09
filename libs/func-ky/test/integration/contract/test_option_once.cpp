//! This file implements testing for the contract between the option type and the owned
//! callables it maps through

#include <gtest/gtest.h>

#include <memory>
#include <utility>

import gbox.func_ky;

using func::make_once;
using option::None;
using option::Option;
using option::Some;

namespace {

/// A value that cannot be copied, so a map which copies where it should have moved fails
/// to build rather than passing quietly
using Owned = std::unique_ptr<int>;

/// The value a test puts into an option, so that reading one back names which it was
constexpr int HELD = 42;

}  // namespace

/// Map a spent option and verify the value it held was transformed
TEST(optionMapSpends, optionOnceTests) {
    Option<int> opt = Some(HELD);

    Option<int> mapped = std::move(opt).map(make_once([](int held) { return held + 1; }));

    ASSERT_EQ(HELD + 1, mapped.as_ref());
}

/// Map a borrowed option and verify it still holds what it already held
TEST(optionMapBorrows, optionOnceTests) {
    Option<int> opt = Some(HELD);

    Option<int> mapped = opt.map(make_once([](const int &held) { return held + 1; }));

    ASSERT_EQ(HELD + 1, mapped.as_ref());
    ASSERT_TRUE(opt.is_some());
    ASSERT_EQ(HELD, opt.as_ref());
}

/// Verify a map changes the type the option holds
TEST(optionMapChangesType, optionOnceTests) {
    Option<int> opt = Some(HELD);

    Option<bool> mapped =
        std::move(opt).map(make_once([](int held) { return held == HELD; }));

    ASSERT_TRUE(mapped.as_ref());
}

/// Verify mapping an empty option leaves the callable uninvoked
TEST(optionMapNoneSkipsCallable, optionOnceTests) {
    Option<int> opt = None();
    bool invoked = false;

    Option<int> mapped = std::move(opt).map(make_once([&invoked](int held) {
        invoked = true;
        return held;
    }));

    ASSERT_TRUE(mapped.is_none());
    ASSERT_FALSE(invoked);
}

/// Verify a map hands a move-only value to the callable rather than copying it
TEST(optionMapMoveOnly, optionOnceTests) {
    Option<Owned> opt = Some(std::make_unique<int>(HELD));

    Option<int> mapped = std::move(opt).map(make_once([](Owned held) { return *held; }));

    ASSERT_EQ(HELD, mapped.as_ref());
}
