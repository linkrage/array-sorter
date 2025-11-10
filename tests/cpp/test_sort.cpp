#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "sort.h"

#include <algorithm>
#include <random>
#include <vector>

namespace {

using Catch::Approx;

bool is_sorted_non_decreasing(const std::vector<double>& values) {
    return std::is_sorted(values.begin(), values.end());
}

}  // namespace

TEST_CASE("sort_array handles empty input", "[sort]") {
    std::vector<double> arr;
    sort_array(arr);
    REQUIRE(arr.empty());
}

TEST_CASE("sort_array leaves a single element unchanged", "[sort]") {
    std::vector<double> arr = {42.0};
    sort_array(arr);
    REQUIRE(arr.size() == 1);
    REQUIRE(arr[0] == 42.0);
}

TEST_CASE("sort_array keeps already sorted data intact", "[sort]") {
    std::vector<double> arr = {1.0, 2.0, 3.0, 4.0, 5.0};
    const auto expected = arr;
    sort_array(arr);
    REQUIRE(arr == expected);
}

TEST_CASE("sort_array fixes reverse sorted sequences", "[sort]") {
    std::vector<double> arr = {5.0, 4.0, 3.0, 2.0, 1.0};
    sort_array(arr);
    REQUIRE(is_sorted_non_decreasing(arr));
    REQUIRE(arr.front() == Approx(1.0));
    REQUIRE(arr.back() == Approx(5.0));
}

TEST_CASE("sort_array sorts arbitrary small data sets", "[sort]") {
    std::vector<double> arr = {3.5, 1.2, 4.7, 2.1, 5.9};
    sort_array(arr);
    REQUIRE(is_sorted_non_decreasing(arr));
}

TEST_CASE("sort_array handles duplicates", "[sort]") {
    std::vector<double> arr = {3.0, 1.0, 3.0, 2.0, 1.0};
    sort_array(arr);
    REQUIRE(is_sorted_non_decreasing(arr));
    REQUIRE(arr[0] == Approx(1.0));
    REQUIRE(arr[1] == Approx(1.0));
    REQUIRE(arr[2] == Approx(2.0));
}

TEST_CASE("sort_array sorts negative values", "[sort]") {
    std::vector<double> arr = {-5.0, 3.0, -1.0, 0.0, 2.0};
    sort_array(arr);
    REQUIRE(is_sorted_non_decreasing(arr));
    REQUIRE(arr.front() == Approx(-5.0));
    REQUIRE(arr[1] == Approx(-1.0));
}

TEST_CASE("sort_array sorts large random inputs", "[sort][stress]") {
    constexpr size_t size = 10'000;
    std::vector<double> arr(size);
    std::mt19937 gen(1337);
    std::uniform_real_distribution<double> dist(-1'000.0, 1'000.0);

    for (double& value : arr) {
        value = dist(gen);
    }

    sort_array(arr);
    REQUIRE(is_sorted_non_decreasing(arr));
}
