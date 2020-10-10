#include <iostream>
#include <cstdint>

using namespace std;

bool IntegerSumOverflow(double lhs, double rhs) {
    auto lhs_u = lhs < 0 ? -lhs : lhs;
    auto rhs_u = rhs < 0 ? -rhs : rhs;
    auto sum_u = lhs_u + rhs_u;

    return sum_u > static_cast<double>(INT64_MAX);
}

int main() {
    int64_t lhs, rhs;

    cin >> lhs >> rhs;

    if (IntegerSumOverflow(lhs, rhs)) {
        cout << "Overflow!" << endl;
        return -1;
    }

    cout << lhs + rhs << endl;
}
