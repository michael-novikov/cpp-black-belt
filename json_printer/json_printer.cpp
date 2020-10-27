#include <test_runner.h>

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <stack>
#include <string>


void PrintJsonString(std::ostream& out, std::string_view str) {
  // реализуйте фукнцию
}

using ArrayContext = void;  // Замените это объявление на определение типа ArrayContext
ArrayContext PrintJsonArray(std::ostream& out) {
  // реализуйте функцию
}

using ObjectContext = void;  // Замените это объявление на определение типа ObjectContext
ObjectContext PrintJsonObject(std::ostream& out) {
  // реализуйте функцию
}

void TestArray() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json
      .Number(5)
      .Number(6)
      .BeginArray()
        .Number(7)
      .EndArray()
      .Number(8)
      .String("bingo!");
  }

  ASSERT_EQUAL(output.str(), R"([5,6,[7],8,"bingo!"])");
}

void TestObject() {
  std::ostringstream output;

  {
    auto json = PrintJsonObject(output);
    json
      .Key("id1").Number(1234)
      .Key("id2").Boolean(false)
      .Key("").Null()
      .Key("\"").String("\\");
  }

  ASSERT_EQUAL(output.str(), R"({"id1":1234,"id2":false,"":null,"\"":"\\"})");
}

void TestAutoClose() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json.BeginArray().BeginObject();
  }

  ASSERT_EQUAL(output.str(), R"([[{}]])");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestArray);
  RUN_TEST(tr, TestObject);
  RUN_TEST(tr, TestAutoClose);

  return 0;
}
