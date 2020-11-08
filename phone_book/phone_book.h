#pragma once

#include "iterator_range.h"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <iosfwd>
#include <map>
#include <set>

struct Date {
  int year, month, day;
};

struct Contact {
  std::string name;
  std::optional<Date> birthday;
  std::vector<std::string> phones;
};

class ContactNameLexicographicComp {
public:
  bool operator()(const Contact& lhs, const Contact& rhs) {
    return lhs.name < rhs.name;
  }
};

class ContactNamePrefixComp {
public:
  bool operator()(const Contact& lhs, std::string_view rhs_name) {
    std::string_view lhs_name{lhs.name};
    auto min_size = rhs_name.size();
    return lhs_name.substr(0, min_size) < rhs_name.substr(0, min_size);
  }
  bool operator()(std::string_view lhs_name, const Contact& rhs) {
    std::string_view rhs_name{rhs.name};
    auto min_size = lhs_name.size();
    return lhs_name.substr(0, min_size) < rhs_name.substr(0, min_size);
  }
};

class PhoneBook {
public:
  using SortedContacts = std::vector<Contact>;
  using SortedContactsIt = SortedContacts::const_iterator;
  using ContactRange = IteratorRange<SortedContactsIt>;

  explicit PhoneBook(std::vector<Contact> contacts);

  ContactRange FindByNamePrefix(std::string_view name_prefix) const;

  void SaveTo(std::ostream& output) const;

private:
   SortedContacts contacts_sorted;
};

PhoneBook DeserializePhoneBook(std::istream& input);

