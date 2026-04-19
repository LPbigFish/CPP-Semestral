#include <algorithm>
#include <cstdint>
#include <print>
#include <vector>

auto main() -> int32_t {
    std::vector<int32_t> v = {5, 3, 1, 4, 2};
    std::ranges::sort(v);
    for (auto x : v) {
        std::println("{}", x);
    }

    std::println("Hello, C++26!");

    return 0;
}
