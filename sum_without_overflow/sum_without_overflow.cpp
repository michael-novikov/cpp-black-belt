#include <iostream>
#include <cstdint>

using namespace std;

bool SumOverflow(int64_t lhs, int64_t rhs) {
    auto lhs_u = static_cast<uint64_t>(lhs < 0 ? -lhs : lhs);
    auto rhs_u = static_cast<uint64_t>(rhs < 0 ? -rhs : rhs);
    auto sum_u = lhs_u + rhs_u;

    return sum_u > static_cast<uint64_t>(INT64_MAX);
}

int main() {
    int64_t lhs, rhs;

    cin >> lhs >> rhs;

    if (SumOverflow(lhs, rhs)) {
        cout << "Overflow!" << endl;
        return -1;
    }

    cout << lhs + rhs << endl;
}
