//! This file implements testing for gbox's result type

#include <gtest/gtest.h>

#include <memory>
#include <type_traits>
#include <utility>

import gbox.func_ky;

using result::Err;
using result::Ok;

enum class ErrorKind : uint8_t { Any, Other };

template <typename T>
using Result = result::Result<T, ErrorKind>;

namespace {

/// A value that cannot be copied, so an operation which copies where it should have
/// moved fails to build rather than passing quietly
using Owned = std::unique_ptr<int32_t>;

/// The value a test puts into a result, so that reading one back names which it was
constexpr int32_t HELD = 42;

/// Whether a result can be formed over `T` and `E` at all
///
/// A requirement only reports unsatisfied rather than erroring when what it names depends
/// on a template parameter, so the type under test is reached through one here.
template <typename T, typename E>
concept Resultable = requires { typename result::Result<T, E>; };

/// Whether a value can be assumed out of a result without spending it
template <typename T>
concept DrainableFromLvalue = requires(Result<T> res) { res.assume_ok(); };

/// Whether a value can be assumed out of a result that the call spends
template <typename T>
concept DrainableFromRvalue = requires(Result<T> res) { std::move(res).assume_ok(); };

/// Whether a result carrying nothing on success offers a map
template <typename T>
concept Mappable = requires(Result<T> res) {
    std::move(res).map(func::make_once([](int32_t held) { return held; }));
};

}  // namespace

// Both sides are checked, and the valid side admits the void a result spells as a
// monostate. Neither side takes a reference, since a result stores what it is given
// unwrapped.
static_assert(Resultable<int32_t, ErrorKind>);
static_assert(Resultable<void, ErrorKind>);
static_assert(Resultable<int32_t, int32_t>);
static_assert(!Resultable<int32_t &, ErrorKind>);
static_assert(!Resultable<int32_t, ErrorKind &>);
static_assert(!Resultable<int32_t, void>);
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
static_assert(!Resultable<int32_t[4], ErrorKind>);

// A move-only value yields a move-only result, and nothing in the type puts a copy back.
static_assert(std::is_move_constructible_v<Result<Owned>>);
static_assert(!std::is_copy_constructible_v<Result<Owned>>);

// Assuming a value spends the result, so a named one cannot be drained silently.
static_assert(DrainableFromRvalue<int32_t>);
static_assert(!DrainableFromLvalue<int32_t>);

// A result carrying nothing on success has no value to transform, and so has no map.
static_assert(Mappable<int32_t>);
static_assert(!Mappable<void>);

TEST(resultOkBuilder, resultTests) {
    Result<int32_t> res = Ok(1);
    ASSERT_TRUE(res.is_ok());
}

TEST(resultErrBuilder, resultTests) {
    Result<int32_t> res = Err(ErrorKind::Any);
    ASSERT_TRUE(res.is_err());
}

TEST(resultOkVoidBuilder, resultTests) {
    Result<void> res = Ok();
    ASSERT_TRUE(res.is_ok());
}

TEST(resultAssumeOk, resultTests) {
    const int32_t valid_value = 42;
    Result<int32_t> res = Ok(valid_value);
    ASSERT_EQ(valid_value, std::move(res).assume_ok());
}

TEST(resultIsOkFalseOnErr, resultTests) {
    Result<int32_t> res = Err(ErrorKind::Any);
    ASSERT_FALSE(res.is_ok());
}

TEST(resultAssumeErrReturnsError, resultTests) {
    Result<int32_t> res = Err(ErrorKind::Any);
    ASSERT_EQ(ErrorKind::Any, std::move(res).assume_err());
}

/// Verify assuming a move-only value hands it over rather than copying it
TEST(resultAssumeOkMoveOnly, resultTests) {
    Result<Owned> res = Ok(std::make_unique<int32_t>(HELD));

    Owned held = std::move(res).assume_ok();

    ASSERT_NE(nullptr, held);
    ASSERT_EQ(HELD, *held);
}

/// Verify assuming a move-only error hands it over rather than copying it
TEST(resultAssumeErrMoveOnly, resultTests) {
    result::Result<int32_t, Owned> res = Err(std::make_unique<int32_t>(HELD));

    Owned held = std::move(res).assume_err();

    ASSERT_NE(nullptr, held);
    ASSERT_EQ(HELD, *held);
}

/// Verify a result whose two sides name the same type still tells them apart
TEST(resultSameTypeBothSides, resultTests) {
    result::Result<int32_t, int32_t> valid = Ok(1);
    result::Result<int32_t, int32_t> erroneous = Err(2);

    ASSERT_TRUE(valid.is_ok());
    ASSERT_FALSE(valid.is_err());
    ASSERT_TRUE(erroneous.is_err());
    ASSERT_FALSE(erroneous.is_ok());

    ASSERT_EQ(1, std::move(valid).assume_ok());
    ASSERT_EQ(2, std::move(erroneous).assume_err());
}

/// Verify assuming a value a result does not hold aborts
TEST(resultAssumeOkOnErrDeathTest, resultTests) {
    Result<int32_t> res = Err(ErrorKind::Any);

    EXPECT_DEATH((void)std::move(res).assume_ok(), "");
}

/// Verify assuming an error a valid result does not hold aborts
TEST(resultAssumeErrOnOkDeathTest, resultTests) {
    Result<int32_t> res = Ok(1);

    EXPECT_DEATH((void)std::move(res).assume_err(), "");
}

/// Verify assuming a value a result carrying nothing does not hold aborts
TEST(resultAssumeOkOnVoidErrDeathTest, resultTests) {
    Result<void> res = Err(ErrorKind::Any);

    EXPECT_DEATH(std::move(res).assume_ok(), "");
}
