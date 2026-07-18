//! This file implements testing for gbox's result type

#include <gtest/gtest.h>

import gbox.core;

using result::Err;
using result::Ok;

enum class ErrorKind { Any };

template <typename T>
using Result = result::Result<T, ErrorKind>;

TEST(resultOkBuilder, resultTests) {
    Result<int> res = Ok(1);
    ASSERT_TRUE(res.is_ok());
}

TEST(resultErrBuilder, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    ASSERT_TRUE(res.is_err());
}

TEST(resultOkVoidBuilder, resultTests) {
    Result<void> res = Ok();
    ASSERT_TRUE(res.is_ok());
}

TEST(resultAssumeOk, resultTests) {
    Result<int> res = Ok(42);
    ASSERT_EQ(42, res.assume_ok());
}

TEST(resultIsOkFalseOnErr, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    ASSERT_FALSE(res.is_ok());
}

TEST(resultAssumeErrReturnsError, resultTests) {
    Result<int> res = Err(ErrorKind::Any);
    ASSERT_EQ(ErrorKind::Any, res.assume_err());
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
