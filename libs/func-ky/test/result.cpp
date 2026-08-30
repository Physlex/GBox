//! This file implements testing for gbox's result type

#include <gtest/gtest.h>

import gbox.func_ky;

using result::Err;
using result::Ok;

enum class ErrorKind : uint8_t { Any };

template <typename T>
using Result = result::Result<T, ErrorKind>;

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
    ASSERT_EQ(valid_value, res.assume_ok());
}

TEST(resultIsOkFalseOnErr, resultTests) {
    Result<int32_t> res = Err(ErrorKind::Any);
    ASSERT_FALSE(res.is_ok());
}

TEST(resultAssumeErrReturnsError, resultTests) {
    Result<int32_t> res = Err(ErrorKind::Any);
    ASSERT_EQ(ErrorKind::Any, res.assume_err());
}

TEST(resultMatchOk, resultTests) {
    Result<int32_t> res = Ok(1);
    int32_t matched = 0;
    result::match_result(
        std::move(res), [&](Ok<int32_t> ok) { matched = ok.value; },
        [&](Err<ErrorKind>) { ADD_FAILURE() << "expected Ok branch"; }
    );

    ASSERT_EQ(1, matched);
}
