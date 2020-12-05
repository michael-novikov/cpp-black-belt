#pragma once

#include <iosfwd>
#include <string>
#include <sstream>
#include <variant>
#include <stdexcept>
#include <optional>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class TestRunner;

namespace Parse {

namespace TokenType {
  struct Number {
    int value;
  };

  struct Id {
    std::string value;
  };

  struct Char {
    char value;
  };

  struct String {
    std::string value;
  };

  struct Class{};
  struct Return{};
  struct If {};
  struct Else {};
  struct Def {};
  struct Newline {};
  struct Print {};
  struct Indent {};
  struct Dedent {};
  struct Eof {};
  struct And {};
  struct Or {};
  struct Not {};
  struct Eq {};
  struct NotEq {};
  struct LessOrEq {};
  struct GreaterOrEq {};
  struct None {};
  struct True {};
  struct False {};
}

using TokenBase = std::variant<
  TokenType::Number,
  TokenType::Id,
  TokenType::Char,
  TokenType::String,
  TokenType::Class,
  TokenType::Return,
  TokenType::If,
  TokenType::Else,
  TokenType::Def,
  TokenType::Newline,
  TokenType::Print,
  TokenType::Indent,
  TokenType::Dedent,
  TokenType::And,
  TokenType::Or,
  TokenType::Not,
  TokenType::Eq,
  TokenType::NotEq,
  TokenType::LessOrEq,
  TokenType::GreaterOrEq,
  TokenType::None,
  TokenType::True,
  TokenType::False,
  TokenType::Eof
>;


//По сравнению с условием задачи мы добавили в тип Token несколько
//удобных методов, которые делают код короче. Например,
//
//token.Is<TokenType::Id>()
//
//гораздо короче, чем
//
//std::holds_alternative<TokenType::Id>(token).
struct Token : TokenBase {
  using TokenBase::TokenBase;

  template <typename T>
  bool Is() const {
    return std::holds_alternative<T>(*this);
  }

  template <typename T>
  const T& As() const {
    return std::get<T>(*this);
  }

  template <typename T>
  const T* TryAs() const {
    return std::get_if<T>(this);
  }
};

bool operator == (const Token& lhs, const Token& rhs);
std::ostream& operator << (std::ostream& os, const Token& rhs);

const std::unordered_map<std::string, Token> keywords = {
  {"class", Token(TokenType::Class{})},
  {"def", Token(TokenType::Def{})},
  {"return", Token(TokenType::Return{})},
  {"print", Token(TokenType::Print{})},
  {"if", Token(TokenType::If{})},
  {"else", Token(TokenType::Else{})},
  {"or", Token(TokenType::Or{})},
  {"and", Token(TokenType::And{})},
  {"not", Token(TokenType::Not{})},
  {"True", Token(TokenType::True{})},
  {"False", Token(TokenType::False{})},
  {"None", Token(TokenType::None{})},
};

const std::unordered_map<std::string, Token> comparison_operators = {
  {"==", Token(TokenType::Eq{})},
  {"!=", Token(TokenType::NotEq{})},
  {"<=", Token(TokenType::LessOrEq{})},
  {">=", Token(TokenType::GreaterOrEq{})},
};

const std::unordered_set<std::string> known_chars = {"=", ".", ",", "(", ")", "+", "-", "<", ">", "*", "/", ":", "?" };

class LexerError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class Lexer {
public:
  explicit Lexer(std::istream& input);

  const Token& CurrentToken() const;
  Token NextToken();

  template <typename T>
  const T& Expect() const {
    if (CurrentToken().Is<T>()) {
        return CurrentToken().As<T>();
    } else {
      throw LexerError{"Lexer expects other token"};
    }
  }

  template <typename T, typename U>
  void Expect(const U& value) const {
    auto& token = Expect<T>();
    if (token.value != value) {
      throw LexerError{"Lexer expects other token"};
    };
  }

  template <typename T>
  const T& ExpectNext() {
    NextToken();
    return Expect<T>();
  }

  template <typename T, typename U>
  void ExpectNext(const U& value) {
    NextToken();
    return Expect<T>(value);
  }

private:
  using IndentationSize = size_t;

  std::istream& input_;
  IndentationSize current_indentation_{0};
  std::string line_;
  std::string_view current_;
  Token token_;

  std::string_view CurrentLine();
  std::string_view NextLine();

  Token NextTokenImpl();

  std::optional<Token> ParseNumber();
  std::optional<Token> ParseKeyword();
  std::optional<Token> ParseId();
  std::optional<Token> ParseComparison();
  std::optional<Token> ParseKnownChar();
  std::optional<Token> ParseQuotedStringLiteral();
};

void RunLexerTests(TestRunner& test_runner);

} /* namespace Parse */
