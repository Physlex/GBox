//! This file implements testing for gbox's result type

#include "gbox/core/result.hpp"

#include <gtest/gtest.h>

using namespace gbox;
using gbox::result::Err;
using gbox::result::Ok;

/// Test error kind
enum class ErrorKind {
    /// Test failure variant
    Any
};

/// Test result alias
template <typename T>
using Result = result::Result<T, ErrorKind>;

/// Test to see if we can explicitly build a result of
/// a given type using the Ok builder
TEST(resultOkBuilder, resultTests) { Result<int> res = Ok(1); }

/// Test to see if we can explicitly build a result of
/// a given type using the Err builder
TEST(resultErrBuilder, resultTests) { Result<int> res = Err(ErrorKind::Any); }

/// Test to see if we can explicitly build a result of
/// a void type using the Err builder
TEST(resultOkVoidBuilder, resultTests) { Result<void> res = Ok(); }

/// Test to see if we can unwrap an Ok value result with no throw
TEST(resultUnwrapOk, resultTests) {
    Result<int> res = Ok(1);
    ASSERT_EQ(1, res.unwrap());
}

/// Test to see if we can unwrap a void type value result with
/// no throw
TEST(resultUnwrapOkVoid, resultTests) {
    Result<void> res = Ok();
    res.unwrap();
}

/// Test to see if we can unwrap an error type result with no throw
TEST(resultUnwrapErr, resultTests) {
    Result<void> res = Err(ErrorKind::Any);
    res.unwrap_err();
}

TEST(resultErrVoidBuilder, resultTests) {
    Result<void> res = Err(ErrorKind::Any);
    ASSERT_TRUE(res.is_err());
}

TEST(resultIsOkTrue, resultTests) {
    Result<int> res = Ok(1);
    ASSERT_TRUE(res.is_ok());
}

TEST(resultIsOkFalse, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    ASSERT_FALSE(res.is_ok());
}

TEST(resultIsErrTrue, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    ASSERT_TRUE(res.is_err());
}

TEST(resultIsErrFalse, resultTests) {
    Result<int> res = Ok(1);
    ASSERT_FALSE(res.is_err());
}

TEST(resultIsOkVoid, resultTests) {
    Result<void> res = Ok();
    ASSERT_TRUE(res.is_ok());
}

TEST(resultUnwrapErrValue, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    ASSERT_EQ(ErrorKind::Any, res.unwrap_err());
}

TEST(resultUnwrapOnErrThrows, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    ASSERT_THROW(res.unwrap(), std::bad_variant_access);
}

TEST(resultUnwrapErrOnOkThrows, resultTests) {
    Result<int> res = Ok(1);
    ASSERT_THROW(res.unwrap_err(), std::bad_variant_access);
}

TEST(resultMatchOk, resultTests) {
    Result<int> res = Ok(1);
    int matched = 0;
    result::match_result(
        std::move(res), [&](Ok<int> ok) { matched = ok.value; },
        [&](Err<ErrorKind>) { ADD_FAILURE() << "expected Ok branch"; }
    );
    ASSERT_EQ(1, matched);
}

TEST(resultMatchErr, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    bool err_called = false;
    result::match_result(
        std::move(res), [&](Ok<int>) { ADD_FAILURE() << "expected Err branch"; },
        [&](Err<ErrorKind> err) {
            err_called = true;
            ASSERT_EQ(ErrorKind::Any, err.value);
        }
    );
    ASSERT_TRUE(err_called);
}
