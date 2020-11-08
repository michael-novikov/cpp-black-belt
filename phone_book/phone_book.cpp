#include "phone_book.h"
#include "contact.pb.h"

#include <iterator>
#include <utility>
#include <algorithm>

using namespace std;

PhoneBook::PhoneBook(std::vector<Contact> contacts)
  : contacts_sorted(move(contacts))
{
  sort(begin(contacts_sorted), end(contacts_sorted), ContactNameLexicographicComp{});
}

PhoneBook::ContactRange PhoneBook::FindByNamePrefix(std::string_view name_prefix) const {
  if (name_prefix == "") {
    return ContactRange{contacts_sorted.begin(), contacts_sorted.end()};
  }

  auto range = equal_range(begin(contacts_sorted), end(contacts_sorted), name_prefix, ContactNamePrefixComp{});
  return ContactRange{range.first, range.second};
}

void PhoneBook::SaveTo(std::ostream& output) const {
  PhoneBookSerialize::ContactList list;
  for (const auto& contact : contacts_sorted) {
    auto contact_serialized = list.add_contact();
    contact_serialized->set_name(contact.name);
    if (contact.birthday.has_value()) {
      const auto& birthday = contact.birthday.value();
      auto birthday_serialized = contact_serialized->mutable_birthday();
      birthday_serialized->set_year(birthday.year);
      birthday_serialized->set_month(birthday.month);
      birthday_serialized->set_day(birthday.day);
    }
    for (const auto& number : contact.phones) {
      contact_serialized->add_phone_number(number);
    }
  }
  list.SerializeToOstream(&output);
}

PhoneBook DeserializePhoneBook(std::istream& input) {
  PhoneBookSerialize::ContactList list;
  list.ParseFromIstream(&input);

  std::vector<Contact> contacts;
  for (const auto& contact : list.contact()) {
    auto name = contact.name();
    optional<Date> birthday{nullopt};
    if (contact.has_birthday()) {
      auto b = contact.birthday();
      birthday = Date{b.year(), b.month(), b.day()};
    }
    vector<string> phones;
    for (const auto& phone : contact.phone_number()) {
      phones.push_back(phone);
    }
    contacts.push_back(Contact{name, birthday, phones});
  }

  return PhoneBook{contacts};
}
