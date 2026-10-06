//! This file implements unit testing for the option type

#include <gtest/gtest.h>

#include <memory>
#include <type_traits>
#include <utility>

import gbox.funky;

using option::None;
using option::Option;
using option::Some;

namespace {

/// A value that cannot be copied, so an operation which copies where it should have
/// moved fails to build rather than passing quietly
using Owned = std::unique_ptr<int>;

/// The value a test puts into an option, so that reading one back names which it was
constexpr int HELD = 42;

/// A second value, for tests that must tell an overwrite from the original
constexpr int OTHER = 7;

/// Whether an option can be formed over `T` at all
///
/// A requirement only reports unsatisfied rather than erroring when what it names depends
/// on a template parameter, so the type under test is reached through one here.
template <typename T>
concept Optionable = requires { typename Option<T>; };

/// Whether a value can be assumed out of an option without spending it
template <typename T>
concept DrainableFromLvalue = requires(Option<T> opt) { opt.assume_some(); };

/// Whether a value can be assumed out of an option that the call spends
template <typename T>
concept DrainableFromRvalue = requires(Option<T> opt) { std::move(opt).assume_some(); };

}  // namespace

// An option is set to `None` deliberately or not at all, so there is no state it reaches
// without an author naming one.
static_assert(!std::is_default_constructible_v<Option<int>>);

// A `Some` exists only to be spent making an option, so only a temporary binds.
static_assert(std::is_constructible_v<Option<int>, Some<int> &&>);
static_assert(std::is_constructible_v<Option<int>, None>);
static_assert(!std::is_constructible_v<Option<int>, Some<int> &>);
static_assert(!std::is_constructible_v<Option<int>, const Some<int> &>);

// A move-only value yields a move-only option, and nothing in the type puts a copy back.
static_assert(std::is_move_constructible_v<Option<Owned>>);
static_assert(!std::is_copy_constructible_v<Option<Owned>>);

// Taking is only as noexcept as moving out what it hands back.
static_assert(noexcept(std::declval<Option<int> &>().take()));

// What a variant is able to hold as an alternative, which is what an option narrows.
static_assert(traits::UnionStorable<int>);
static_assert(traits::UnionStorable<int &>);
static_assert(!traits::UnionStorable<void>);
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
static_assert(!traits::UnionStorable<int[4]>);

// What an option may hold: any non-array object type, and a reference to one. `None` is
// the empty case itself, never something the filled case carries.
static_assert(Optionable<int>);
static_assert(Optionable<int &>);
static_assert(Optionable<Option<int>>);
static_assert(!Optionable<void>);
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
static_assert(!Optionable<int[4]>);
static_assert(!Optionable<None>);

// Assuming a value spends the option, so a named one cannot be drained silently.
static_assert(DrainableFromRvalue<int>);
static_assert(!DrainableFromLvalue<int>);

// An option holding a reference lends out a referent that can still be written to, even
// when the option itself cannot be.
static_assert(
    std::is_same_v<decltype(std::declval<const Option<int &> &>().as_ref()), int &>
);

/// Construct an option of `Some` type and verify it is indeed a `Some` type
TEST(optionSomeBuilder, optionTests) {
    Option<int> opt = Some(HELD);

    ASSERT_TRUE(opt.is_some());
    ASSERT_FALSE(opt.is_none());
}

/// Construct an option of `None` type and verify it is indeed a `None` type
TEST(optionNoneBuilder, optionTests) {
    Option<int> opt = None();

    ASSERT_TRUE(opt.is_none());
    ASSERT_FALSE(opt.is_some());
}

/// Verify a borrow leaves the value in the option and names the same storage each time
TEST(optionAsRefBorrows, optionTests) {
    Option<int> opt = Some(HELD);

    ASSERT_EQ(HELD, opt.as_ref());
    ASSERT_EQ(&opt.as_ref(), &opt.as_ref());
    ASSERT_TRUE(opt.is_some());
}

/// Verify a write through `as_mut` is visible to a later borrow
TEST(optionAsMutWritesThrough, optionTests) {
    Option<int> opt = Some(HELD);

    opt.as_mut() = OTHER;

    ASSERT_EQ(OTHER, opt.as_ref());
}

/// Assume a value and verify it comes back out
TEST(optionAssumeSome, optionTests) {
    Option<int> opt = Some(HELD);

    ASSERT_EQ(HELD, std::move(opt).assume_some());
}

/// Verify assuming a move-only value hands it over rather than copying it
TEST(optionAssumeSomeMoveOnly, optionTests) {
    Option<Owned> opt = Some(std::make_unique<int>(HELD));

    Owned held = std::move(opt).assume_some();

    ASSERT_NE(nullptr, held);
    ASSERT_EQ(HELD, *held);
}

/// Take a value and verify the option it came from is left empty
TEST(optionTakeSome, optionTests) {
    Option<int> opt = Some(HELD);

    Option<int> taken = opt.take();

    ASSERT_TRUE(taken.is_some());
    ASSERT_EQ(HELD, taken.as_ref());
    ASSERT_TRUE(opt.is_none());
}

/// Verify taking from an empty option yields an empty option rather than aborting
TEST(optionTakeNone, optionTests) {
    Option<int> opt = None();

    Option<int> taken = opt.take();

    ASSERT_TRUE(taken.is_none());
    ASSERT_TRUE(opt.is_none());
}

/// Verify taking a move-only value carries it into the returned option
TEST(optionTakeMoveOnly, optionTests) {
    Option<Owned> opt = Some(std::make_unique<int>(HELD));

    Option<Owned> taken = opt.take();

    ASSERT_TRUE(opt.is_none());
    ASSERT_EQ(HELD, *taken.as_ref());
}

/// Verify an option over a reference binds the value it was given rather than copying it
TEST(optionRefBinds, optionTests) {
    int source = HELD;
    Option<int &> opt = Some<int &>(source);

    ASSERT_EQ(&source, &opt.as_ref());
}

/// Verify a write through a bound reference reaches the value it named
TEST(optionRefWritesThrough, optionTests) {
    int source = HELD;
    Option<int &> opt = Some<int &>(source);

    opt.as_mut() = OTHER;

    ASSERT_EQ(OTHER, source);
}

/// Verify taking a bound reference carries the binding into the returned option
TEST(optionRefTake, optionTests) {
    int source = HELD;
    Option<int &> opt = Some<int &>(source);

    Option<int &> taken = opt.take();

    ASSERT_TRUE(opt.is_none());
    ASSERT_EQ(&source, &taken.as_ref());
}

/// Verify assuming a bound reference returns the value it named, not a copy of it
TEST(optionRefAssumeSome, optionTests) {
    int source = HELD;
    Option<int &> opt = Some<int &>(source);

    int &held = std::move(opt).assume_some();

    ASSERT_EQ(&source, &held);
}

/// Verify an option holding a reference lends out a writable referent even when the
/// option itself cannot be written to
TEST(optionRefConstLendsMutable, optionTests) {
    int source = HELD;
    const Option<int &> opt = Some<int &>(source);

    opt.as_ref() = OTHER;

    ASSERT_EQ(OTHER, source);
}

/// Verify asserting emptiness on an empty option does nothing
TEST(optionAssumeNone, optionTests) {
    Option<int> opt = None();

    opt.assume_none();

    ASSERT_TRUE(opt.is_none());
}

/// Verify assuming a value an option does not hold aborts
TEST(optionAssumeSomeOnNoneDeathTest, optionTests) {
    Option<int> opt = None();

    EXPECT_DEATH((void)std::move(opt).assume_some(), "");
}

/// Verify borrowing from an option that holds nothing aborts
TEST(optionAsRefOnNoneDeathTest, optionTests) {
    Option<int> opt = None();

    EXPECT_DEATH((void)opt.as_ref(), "");
}

/// Verify borrowing mutably from an option that holds nothing aborts
TEST(optionAsMutOnNoneDeathTest, optionTests) {
    Option<int> opt = None();

    EXPECT_DEATH((void)opt.as_mut(), "");
}

/// Verify asserting emptiness on an option that holds a value aborts
TEST(optionAssumeNoneOnSomeDeathTest, optionTests) {
    Option<int> opt = Some(HELD);

    EXPECT_DEATH(opt.assume_none(), "");
}
