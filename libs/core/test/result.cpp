//! This file implements testing for gbox's result type

#include <gtest/gtest.h>

import gbox.core;

using result::Err;
using result::Ok;

/// Test error kind
enum class ErrorKind {
    /// Test failure variant
    Any
};

/// Test result alias
template <typename T>
using Result = result::Result<T, ErrorKind>;

/// Test to see if we can explicitly build a result of a given type using the Ok builder
TEST(resultOkBuilder, resultTests) {
    Result<int> res = Ok(1);
    ASSERT_TRUE(res.is_ok());
}

/// Test to see if we can explicitly build a result of a given type using the Err builder
TEST(resultErrBuilder, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    ASSERT_TRUE(res.is_err());
}

/// Test to see if we can explicitly build a result of a void type using the Ok builder
TEST(resultOkVoidBuilder, resultTests) {
    Result<void> res = Ok();
    ASSERT_TRUE(res.is_ok());
}

/// Test to see if we can retrieve an Ok value with no throw
TEST(resultAssumeOk, resultTests) {
    Result<int> res = Ok(42);
    ASSERT_EQ(42, res.assume_ok());
}

/// Test that an erroneous result does not report itself as Ok
TEST(resultIsOkFalseOnErr, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    ASSERT_FALSE(res.is_ok());
}

/// FAILING: `assume_err()` should return the contained error value. It currently asserts
/// `holds_alternative<T>` (the Ok type) rather than `<E>`, so under enabled asserts it
/// aborts on a genuinely erroneous result instead of returning the error.
TEST(resultAssumeErrReturnsError, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    ASSERT_EQ(ErrorKind::Any, res.assume_err());
}

#if 0
// FIXME(bug): match_result() does not compile. It calls `res.inner()`, but Result has no
// public inner() accessor (`inner_` is private). Re-enable once Result exposes its inner
// variant.
TEST(resultMatchOk, resultTests) {
    Result<int> res = Ok(1);
    int matched = 0;
    result::match_result(
        std::move(res), [&](Ok<int> ok) { matched = ok.value; },
        [&](Err<ErrorKind>) { ADD_FAILURE() << "expected Ok branch"; }
    );
    ASSERT_EQ(1, matched);
}
#endif
