#pragma once
#include <cstdio>
#include <iostream>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (__builtin_expect(!(condition), 0)) {                                   \
      std::cerr << "Assertion: " << __FILE__ << "(" << __func__     \
                << ":" << __LINE__ << ")"                           \
                << ": \'" << #condition << "\' failed" << std::endl;\
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

// check the val1 op val2
#define CHECK_OP(op, val1, val2)                                            \
  do {                                                                      \
    const decltype(val1) _CHECK_OP_v1_ = (decltype(val1))val1;                  \
    const decltype(val2) _CHECK_OP_v2_ = (decltype(val2))val2;                  \
    if (__builtin_expect(!((_CHECK_OP_v1_)op(decltype(val1))(_CHECK_OP_v2_)), \
                         0)) {                                              \
      std::cerr << "Assertion: " << __FILE__ << "(" << __func__  \
                << ":" << __LINE__ << ")"                        \
                << ": \'" << #val1 << " " << #op << " " << #val2 \
                << " [ " << val1 << " " << #op << " " << val2    \
                << " ]\'"                                        \
                << " failed" << std::endl;                       \
      exit(1);                                                              \
    }                                                                       \
  } while (0)

#define CHECK_EQ(val1, val2) CHECK_OP(==, val1, val2)
#define CHECK_NE(val1, val2) CHECK_OP(!=, val1, val2)
#define CHECK_LE(val1, val2) CHECK_OP(<=, val1, val2)
#define CHECK_LT(val1, val2) CHECK_OP(<, val1, val2)
#define CHECK_GE(val1, val2) CHECK_OP(>=, val1, val2)
#define CHECK_GT(val1, val2) CHECK_OP(>, val1, val2)

// condition assert
#ifdef GTEST_INCLUDE_GTEST_GTEST_H_
#undef ASSERT_TRUE
#undef ASSERT_FALSE
#undef ASSERT
#undef ASSERT_EQ
#undef ASSERT_NE
#undef ASSERT_LE
#undef ASSERT_LT
#undef ASSERT_GE
#undef ASSERT_GT
#undef ASSERT_STREQ
#endif

#define ASSERT_TRUE(cond) CHECK(cond)
#define ASSERT_FALSE(cond) CHECK(!(cond))
// adapt to the original wukong assert
#define ASSERT(cond) CHECK(cond)

#define ASSERT_EQ(val1, val2) CHECK_EQ(val1, val2)
#define ASSERT_NE(val1, val2) CHECK_NE(val1, val2)
#define ASSERT_LE(val1, val2) CHECK_LE(val1, val2)
#define ASSERT_LT(val1, val2) CHECK_LT(val1, val2)
#define ASSERT_GE(val1, val2) CHECK_GE(val1, val2)
#define ASSERT_GT(val1, val2) CHECK_GT(val1, val2)

// string equal
#define ASSERT_STREQ(a, b) CHECK(strcmp(a, b) == 0)

// check the condition if wrong print out the message of variable parameters in
// fmt
#define ASSERT_MSG(condition, fmt, ...)                                        \
  do {                                                                         \
    if (__builtin_expect(!(condition), 0)) {                                   \
      std::cerr << "Assertion: " << __FILE__ << "(" << __func__     \
                << ":" << __LINE__ << ")"                           \
                << ": \'" << #condition << "\' failed" << std::endl;\
      fprintf(stderr, fmt, ##__VA_ARGS__);                                     \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define ASSERT_ERROR_CODE(condition, error_code)                     \
    do {                                                               \
        if (__builtin_expect(!(condition), 0)) {                       \
            std::cerr                                       \
                << "Assertion: " << __FILE__ << "(" << __func__ << ":" \
                << __LINE__ << ")"                                     \
                << ": \'" << #condition << "\' failed" << std::endl;   \
            exit(1);                                                   \
        }                                                              \
    } while (0)


// #define ASSERT_MSG(condition, fmt, ...)         \
//     if (!condition) {                           \
//         fprintf(stderr, fmt, ##__VA_ARGS__);    \
//         exit(1);                                \
//     }
