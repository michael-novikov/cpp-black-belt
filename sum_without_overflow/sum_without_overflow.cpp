#include <iostream>
#include <cstdint>
#include <limits>
#include <optional>

using SignedNumber = int64_t;

template <typename T>
std::optional<T> add(T lhs, T rhs) {
    if (lhs > 0 && rhs > 0) {
        if (lhs > std::numeric_limits<T>::max() - rhs) {
            return std::nullopt;
        }
    } else if (lhs < 0 && rhs < 0) {
        if (lhs < std::numeric_limits<T>::min() - rhs) {
            return std::nullopt;
        }
    }
    
    return lhs + rhs;
}

int main() {
    SignedNumber lhs, rhs;

    std::cin >> lhs >> rhs;

    if (auto sum = add(lhs, rhs)) {
        std::cout << sum.value() << std::endl;
    } else {
        std::cout << "Overflow!" << std::endl;
    }
}
