#include "lexer.h"

#include "string_utils.h"

#include <algorithm>
#include <charconv>
#include <exception>
#include <stdexcept>
#include <unordered_map>
#include <string_view>

using namespace std;

namespace Parse {

bool operator == (const Token& lhs, const Token& rhs) {
  using namespace TokenType;

  if (lhs.index() != rhs.index()) {
    return false;
  }
  if (lhs.Is<Char>()) {
    return lhs.As<Char>().value == rhs.As<Char>().value;
  } else if (lhs.Is<Number>()) {
    return lhs.As<Number>().value == rhs.As<Number>().value;
  } else if (lhs.Is<String>()) {
    return lhs.As<String>().value == rhs.As<String>().value;
  } else if (lhs.Is<Id>()) {
    return lhs.As<Id>().value == rhs.As<Id>().value;
  } else {
    return true;
  }
}

std::ostream& operator << (std::ostream& os, const Token& rhs) {
  using namespace TokenType;

#define VALUED_OUTPUT(type) \
  if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

  VALUED_OUTPUT(Number);
  VALUED_OUTPUT(Id);
  VALUED_OUTPUT(String);
  VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

  UNVALUED_OUTPUT(Class);
  UNVALUED_OUTPUT(Return);
  UNVALUED_OUTPUT(If);
  UNVALUED_OUTPUT(Else);
  UNVALUED_OUTPUT(Def);
  UNVALUED_OUTPUT(Newline);
  UNVALUED_OUTPUT(Print);
  UNVALUED_OUTPUT(Indent);
  UNVALUED_OUTPUT(Dedent);
  UNVALUED_OUTPUT(And);
  UNVALUED_OUTPUT(Or);
  UNVALUED_OUTPUT(Not);
  UNVALUED_OUTPUT(Eq);
  UNVALUED_OUTPUT(NotEq);
  UNVALUED_OUTPUT(LessOrEq);
  UNVALUED_OUTPUT(GreaterOrEq);
  UNVALUED_OUTPUT(None);
  UNVALUED_OUTPUT(True);
  UNVALUED_OUTPUT(False);
  UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

  return os << "Unknown token :(";
}


Lexer::Lexer(std::istream& input)
  : input_(input)
{
  NextToken();
}

const Token& Lexer::CurrentToken() const {
  return token_;
}

std::string_view Lexer::CurrentLine() {
  return current_.empty() ? NextLine() : current_;
}

std::string_view Lexer::NextLine() {
  line_.clear();
  while (getline(input_, line_)) {
    if (!CheckEmpty(line_)) {
      break;
    }
  }
  return line_;
}

Token Lexer::NextToken() {
  token_ = NextTokenImpl();
  return token_;
}

#define TryMethod(method) \
  if (auto optional_res = method(); optional_res) { \
    return optional_res.value(); \
  }

Token Lexer::NextTokenImpl() {
  if (current_.empty() && !line_.empty()) {
    line_.clear();
    return Token(TokenType::Newline{});
  }

  current_ = CurrentLine();

  if (current_ == line_) {
    auto space_count = current_.size() - RemovePrefix(current_, ' ').size();
    if (auto indentation = space_count / 2; indentation != current_indentation_) {
      if (indentation > current_indentation_) {
        ++current_indentation_;
        return Token(TokenType::Indent{});
      } else {
        --current_indentation_;
        return Token(TokenType::Dedent{});
      }
    }
  }

  if (current_.empty()) {
    return Token(TokenType::Eof{});
  }

  current_ = RemovePrefix(current_, ' ');

  TryMethod(ParseNumber);
  TryMethod(ParseKeyword);
  TryMethod(ParseId);
  TryMethod(ParseComparison);
  TryMethod(ParseKnownChar);
  TryMethod(ParseQuotedStringLiteral);

  throw std::runtime_error("unrecognized token here: " + string{current_});
}

std::optional<Token> Lexer::ParseNumber() {
  string_view number_token;
  tie(number_token, current_) = GetNumber(current_);
  if (!number_token.empty()) {
    auto value = stoi(string{number_token});
    return Token(TokenType::Number{value});
  }
  return nullopt;
}

std::optional<Token> Lexer::ParseKeyword() {
  string_view keyword_token;
  tie(keyword_token, current_) = GetKnownPattern(current_, keywords);
  if (!keyword_token.empty()) {
    return keywords.at(string{keyword_token});
  }
  return nullopt;
}

std::optional<Token> Lexer::ParseId() {
  string_view id_token;
  tie(id_token, current_) = GetId(current_);
  if (!id_token.empty()) {
    return Token(TokenType::Id{string{id_token}});
  }
  return nullopt;
}

std::optional<Token> Lexer::ParseComparison() {
  string_view operator_token;
  tie(operator_token, current_) = GetKnownPattern(current_, comparison_operators);
  if (!operator_token.empty()) {
    return comparison_operators.at(string{operator_token});
  }
  return nullopt;
}

std::optional<Token> Lexer::ParseKnownChar() {
  string_view char_token;
  tie(char_token, current_) = GetKnownPattern(current_, known_chars);
  if (!char_token.empty()) {
    return Token(TokenType::Char{char_token.front()});
  }
  return nullopt;
}

std::optional<Token> Lexer::ParseQuotedStringLiteral() {
  if (char quote = current_.front(); quote == '\'' || quote == '\"') {
    string_view string_literal;
    tie(string_literal, current_) = GetQuotedStringLiteral(current_, quote);
    return Token(TokenType::String{string{string_literal}});
  }
  return nullopt;
}

} /* namespace Parse */
