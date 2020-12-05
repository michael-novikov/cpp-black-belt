#include "string_utils.h"

#include <cctype>
#include <string_view>

using namespace std;

bool CheckEmpty(std::string_view str, char ignore) {
  return RemovePrefix(str, ignore).empty();
};

std::string_view RemovePrefix(std::string_view str, char to_remove) {
  str.remove_prefix(min(str.find_first_not_of(to_remove), str.size()));
  return str;
}

std::pair<std::string_view, std::string_view> GetId(std::string_view str) {
  if (str.front() == '_' || isalpha(static_cast<unsigned char>(str.front()))) {
    size_t pos = 1;
    while (IsAllowedForId(str[pos])) {
      ++pos;
    }
    return {str.substr(0, pos), str.substr(pos)};
  }

  return {string_view{}, str};
}

bool IsAllowedForId(char ch) {
  return ch == '_' || isalnum(static_cast<unsigned char>(ch));
}

std::pair<std::string_view, std::string_view> GetNumber(std::string_view str) {
  return SplitTwo(str, [](unsigned char c) { return !isdigit(c); });
}

std::pair<std::string_view, std::string_view> GetQuotedStringLiteral(std::string_view str, char quote) {
  if (str.empty() || str.front() != quote) {
    return {string_view{}, str};
  }
  auto pos = str.find(quote, 1);
  return {str.substr(1, pos - 1), str.substr(pos + 1)};
}

SplittedViews GetKnownPattern(std::string_view str, const std::unordered_set<std::string>& patterns) {
  for (const auto& pattern : patterns) {
    if (auto pos = str.find(pattern); pos == 0) {
      return {str.substr(0, pattern.length()), str.substr(pattern.length())};
    }
  }
  return {std::string_view{}, str};
}

