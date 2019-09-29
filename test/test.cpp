#include "../expression.hpp"

#define CATCH_CONFIG_MAIN

#if __has_include(<catch2/catch.hpp>)
#   include <catch2/catch.hpp>
#elif __has_include("catch.hpp")
#   include "catch.hpp"
#else
#   error "Run cmake or download catch.hpp manually"
#endif

#include <algorithm>
#include <array>
#include <vector>
#include <iterator>

using namespace expr;

TEST_CASE("sort", "[placeholder]")
{
    std::vector<int> v{ 3, 5, 1, 7, 2, 4, 6 };
    std::sort(begin(v), end(v), _1 > _2);

    REQUIRE(v == std::vector<int>{ 7, 6, 5, 4, 3, 2, 1 });
}

TEST_CASE("transform", "[placeholder]")
{
    std::vector<int> a{ 1, 2, 3 };
    std::vector<int> b{ 4, 5, 6 };
    std::vector<int> c(3);
    std::transform(begin(a), end(a), begin(b), begin(c), _1 + _2);

    REQUIRE(c == std::vector{ 5, 7, 9 });
}

TEST_CASE("sum", "[placeholder]")
{
    std::vector<int> v{ 1, 2, 3 };
    int sum = 0;
    std::for_each(begin(v), end(v), sum += _1);

    REQUIRE(sum == 6);
}

TEST_CASE("mem-obj", "[placeholder]")
{
    struct A { int i; } a;
    auto lambda = (_1->*&A::i) = _2;
    lambda(a, 5);

    REQUIRE(a.i == 5);
}

TEST_CASE("mem-fun", "[placeholder]")
{
    struct A { int get(char, float) { return 5; } };
    auto lambda = (_1->*&A::get)(_2, 3.14f);
    const int res = lambda(A{}, 'a');

    REQUIRE(res == 5);
}

TEST_CASE("indices", "[placeholder]")
{
    std::array<int, 5> arr{ 1, 2, 3, 4, 5 };
    std::array<int, 5> indices{ 4, 3, 2, 1, 0 };
    std::for_each(begin(indices), end(indices), ++variable(arr)[_1]);

    REQUIRE(arr == std::array<int, 5>{ 2, 3, 4, 5, 6 });
}

TEST_CASE("ptr-offsets", "[placeholder]")
{
    int arr[] = { 1, 2, 3, 4, 5 };
    auto lambda = *(_1 + 1);

    REQUIRE(lambda(arr) == 2 );
    REQUIRE(lambda(arr + 1) == 3);
    REQUIRE(lambda(arr + 2) == 4);
    REQUIRE(lambda(arr + 3) == 5);
}

TEST_CASE("short-circuit", "[placeholder]")
{
    int i = 0;
    (_1 || ++_2)(true, i);
    REQUIRE(i == 0);
}
