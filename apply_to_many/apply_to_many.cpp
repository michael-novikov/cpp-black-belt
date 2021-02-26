#include "test_runner.h"

// ApplyToMany(f, 1, 2, 3) означает f(1); f(2); f(3);

void TestSum() {
  int x;
  auto lambda = [&x](int y) { x += y; };

  x = 0;
  ApplyToMany(lambda, 1);
  ASSERT_EQUAL(x, 1);

  x = 0;
  ApplyToMany(lambda, 1, 2, 3, 4, 5);
  ASSERT_EQUAL(x, 15);
}

void TestConcatenate() {
  std::string s;
  auto lambda = [&s](const auto& t) {
    if (!s.empty()) {
      s += " ";
    }
    s += t;
  };

  ApplyToMany(lambda, "cyan", "magenta", "yellow", "black");
  ASSERT_EQUAL(s, "cyan magenta yellow black");
}

void TestIncrement() {
  auto increment = [](int& x) { ++x; };

  int a = 0;
  int b = 3;
  int c = 43;

  ApplyToMany(increment, a, b, c);
  ASSERT_EQUAL(a, 1);
  ASSERT_EQUAL(b, 4);
  ASSERT_EQUAL(c, 44);
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestSum);
  RUN_TEST(tr, TestConcatenate);
  RUN_TEST(tr, TestIncrement);
  return 0;
}
