// ++C Runtime Library (xxCRT)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

#ifndef TEST_H
#define TEST_H

#include <cio.h>

#define ASSERT(cond) \
    do { \
        if (!(cond)) \
        { \
            printf("  [FAIL] %s:%d: Assertion '%s' failed.\n", __FILE__, __LINE__, #cond); \
            return false; \
        } \
    } while (0)

#define ASSERT_STR_EQ(a, b) \
    do { \
        if (strcmp((a), (b)) != 0) \
        { \
            printf("  [FAIL] %s:%d: Expected '%s', got '%s'.\n", __FILE__, __LINE__, (b), (a)); \
            return false; \
        } \
    } while (0)

#define RUN_TEST(test) \
    do { \
        printf("Running %s...\n", #test); \
        g_tests_run++; \
        if (test()) \
        { \
            printf("  [OK]\n"); \
            g_tests_passed++; \
        } \
    } while (0)

#endif /* TEST_H */