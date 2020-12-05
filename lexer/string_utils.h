#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <unordered_set>

bool CheckEmpty(std::string_view str, char ignore = ' ');
std::string_view RemovePrefix(std::string_view str, char to_remove);

template <typename UnaryPredicate>
std::pair<std::string_view, std::optional<std::string_view>> SplitTwoStrict(std::string_view str, UnaryPredicate p) {
  auto it = std::find_if(begin(str), end(str), p);
  if (it == end(str)) {
    return {str, std::nullopt};
  }

  size_t pos = it - begin(str);
  return {str.substr(0, pos), str.substr(pos)};
}

using SplittedViews = std::pair<std::string_view, std::string_view>;

template <typename UnaryPredicate>
SplittedViews SplitTwo(std::string_view str, UnaryPredicate p) {
  const auto [lhs, rhs_opt] = SplitTwoStrict(str, p);
  return {lhs, rhs_opt.value_or("")};
}

SplittedViews GetId(std::string_view str);
SplittedViews GetNumber(std::string_view str);
SplittedViews GetQuotedStringLiteral(std::string_view str, char quote = '\'');

template <typename Token>
SplittedViews GetKnownPattern(std::string_view str, const std::unordered_map<std::string, Token>& patterns) {
  for (const auto& [pattern, _] : patterns) {
    if (auto pos = str.find(pattern); pos == 0) {
      return {str.substr(0, pattern.length()), str.substr(pattern.length())};
    }
  }
  return {std::string_view{}, str};
}

SplittedViews GetKnownPattern(std::string_view str, const std::unordered_set<std::string>& patterns);

bool IsAllowedForId(char ch);

