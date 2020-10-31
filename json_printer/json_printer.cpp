#include <ios>
#include <ostream>
#include <string_view>
#include <test_runner.h>

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <stack>
#include <string>
#include <cstdint>

class EmptyContext {};

template <typename Parent>
class Array;
template <typename Parent>
class Object;

template <typename Parent>
class Value {
public:
  Value(std::ostream& out) : out_(out) {}
  Value(std::ostream &out, Parent *parent_context)
      : out_(out), parent_(parent_context) {}
  ~Value() { if (!printed_) Null(); }

  Parent &Number(int64_t value);
  Parent &String(std::string_view value);
  Parent &Boolean(bool value);
  Parent &Null();

  Array<Parent> BeginArray() { printed_ = true; return {out_, this->parent_}; }
  Object<Parent> BeginObject() { printed_ = true; return {out_, this->parent_}; }

private:
  std::ostream& out_;
  Parent *parent_{nullptr};
  bool printed_{false};
};

template <typename Parent>
Parent& Value<Parent>::Number(int64_t value) {
  out_ << value;
  printed_ = true;
  return *parent_;
}

template <typename Parent>
Parent& Value<Parent>::String(std::string_view value) {
  out_ << '\"';

  for (auto ch : value) {
    if (ch == '\"' || ch == '\\') {
      out_ << '\\';
    }
    out_ << ch;
  }

  out_ << '\"';

  printed_ = true;
  return *parent_;
}

template <typename Parent>
Parent& Value<Parent>::Boolean(bool value) {
  out_ << std::boolalpha << value;
  printed_ = true;
  return *parent_;
}

template <typename Parent>
Parent& Value<Parent>::Null() {
  out_ << "null";
  printed_ = true;
  return *parent_;
}

template <typename Parent, char BeginChar, char EndChar, char DelimeterChar>
class Context {
public:
  Context(std::ostream& out) : out_(out) { Begin(); }
  Context(std::ostream &out, Parent *context) : out_(out), parent_(context) { Begin(); }
  ~Context() { End(); }

protected:
  std::ostream& out_;
  Parent *parent_{nullptr};
  bool first_added_{false};
  bool opened_{false};

  Value<Context> AddValue() { Delimeter(); return Value<Context>{out_, this}; }

  void Begin() {
    out_ << BeginChar;
    opened_ = true;
  }

  void Delimeter() {
    if (!first_added_) {
      first_added_ = true;
    } else {
      out_ << ',';
    }
  }

  void End() {
    if (opened_) {
      out_ << EndChar;
      opened_ = false;
    }
  }
};

template<typename Parent>
class Array : public Context<Parent, '[', ']', ','> {
public:
  Array(std::ostream& out) : Context<Parent, '[', ']', ','>(out) {}
  Array(std::ostream& out, Parent* parent) : Context<Parent, '[', ']', ','>(out, parent) {}

  Array<Parent>& Number(int64_t value) { return this->AddValue().Number(value); return *this; }
  Array<Parent>& String(std::string_view value) { this->AddValue().String(value); return *this; }
  Array<Parent>& Boolean(bool value) { this->AddValue().Boolean(value); return *this; }
  Array<Parent>& Null() { this->AddValue().Null(); return *this; }

  Array<Array<Parent>> BeginArray() { this->Delimeter(); return {this->out_, this}; }
  Object<Array<Parent>> BeginObject() { this->Delimeter(); return {this->out_, this}; }

  Parent& EndArray() { this->End(); return *this->parent_; }
};

template<typename Parent>
class Object : public Context<Parent, '{', '}', ','> {
public:
  Object(std::ostream& out) : Context<Parent, '{', '}', ','>(out) {}
  Object(std::ostream& out, Parent* parent) : Context<Parent, '{', '}', ','>(out, parent) {}

  Value<Object<Parent>> Key(std::string_view value) {
    this->Delimeter();
    Value<EmptyContext>{this->out_}.String(value);
    this->out_ << ':';
    return {this->out_, this};
  }

  Parent& EndObject() { this->End(); return *this->parent_; }
};

void PrintJsonString(std::ostream& out, std::string_view str) {
  Value<EmptyContext>{out}.String(str);
}

using ArrayContext = Array<EmptyContext>;
ArrayContext PrintJsonArray(std::ostream& out) {
  return {out};
}

using ObjectContext = Object<EmptyContext>;
ObjectContext PrintJsonObject(std::ostream& out) {
  return {out};
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

void TestArrayAfterEnd() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json
      .EndArray();
  }

  ASSERT_EQUAL(output.str(), R"([])");
}

void TestBigArray() {
  std::ostringstream output;
  {
    auto json = PrintJsonArray(output);
    json
      .Number(5)
      .Number(-64)
      .BeginArray()
        .BeginObject()
           .Key("").Number(6)
        .EndObject()
        .Number(7)
      .EndArray()
      .Null()
      .Boolean(false)
      .BeginObject()
        .Key("").Number(6)
        .Key("id2").Boolean(true)
        .Key("").Null()
        .Key("\"").String("\\")
      .EndObject()
      .Number(8)
      .String("bingo!")
        .BeginObject()
          .Key("123")
          .Null();
  }
  ASSERT_EQUAL(output.str(), R"([5,-64,[{"":6},7],null,false,{"":6,"id2":true,"":null,"\"":"\\"},8,"bingo!",{"123":null}])");
}

void TestEmptyArray() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json
      .BeginArray()
      .BeginObject()
      .EndObject()
      .BeginObject()
      .EndObject()
      .EndArray()

      .BeginArray()
      .BeginObject()
      .EndObject()
      .BeginObject()
      .EndObject()
      .EndArray();
  }

  ASSERT_EQUAL(output.str(), R"([[{},{}],[{},{}]])");
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

void TestObjectAfterEnd() {
  std::ostringstream output;

  {
    auto json = PrintJsonObject(output);
    json
      .EndObject();
  }

  ASSERT_EQUAL(output.str(), R"({})");
}

void TestNullIfKeyOnly() {
  std::ostringstream output;

  {
    auto json = PrintJsonObject(output);

    json
      .Key("foo")
      .BeginArray()
         .String("Hello")
      .EndArray()
      .Key("foo")  // повторяющиеся ключи допускаются
      .BeginObject()
        .Key("foo");  // закрытие объекта в таком состоянии допишет null в качестве значения
  }
  ASSERT_EQUAL(output.str(), R"({"foo":["Hello"],"foo":{"foo":null}})");
}

void TestAutoClose() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json.BeginArray().BeginObject();
  }

  ASSERT_EQUAL(output.str(), R"([[{}]])");
}

void TestPrintString() {
  std::ostringstream output;

  {
    PrintJsonString(output, "Hello, \"world\"");
  }

  ASSERT_EQUAL(output.str(), R"("Hello, \"world\"")");
}

void TestExplicitlyClosedArray() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json
      .Null()
      .String("Hello")
      .Number(123)
      .Boolean(false)
      .EndArray();
  }

  ASSERT_EQUAL(output.str(), R"([null,"Hello",123,false])");
}

void TestArraySimple() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json
      .Null()
      .String("Hello")
      .Number(123)
      .Boolean(false);
  }

  ASSERT_EQUAL(output.str(), R"([null,"Hello",123,false])");
}

void TestImplicitlyClosedArray() {
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json
      .String("Hello")
      .BeginArray()
        .String("World");
  }

  ASSERT_EQUAL(output.str(), R"(["Hello",["World"]])");
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestArray);
  RUN_TEST(tr, TestBigArray);
  RUN_TEST(tr, TestEmptyArray);
  RUN_TEST(tr, TestObject);
  RUN_TEST(tr, TestObjectAfterEnd);
  RUN_TEST(tr, TestNullIfKeyOnly);
  RUN_TEST(tr, TestAutoClose);
  RUN_TEST(tr, TestPrintString);
  RUN_TEST(tr, TestExplicitlyClosedArray);
  RUN_TEST(tr, TestArraySimple);
  RUN_TEST(tr, TestImplicitlyClosedArray);

  return 0;
}
