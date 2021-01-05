#include "object.h"
#include "object_holder.h"
#include "statement.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string_view>
#include <utility>

using namespace std;

namespace Runtime {

void ClassInstance::Print(std::ostream& os) {
  if (HasMethod("__str__", 0)) {
    Call("__str__", {})->Print(os);
  } else {
    os << "0x" << hex << reinterpret_cast<size_t>(this) << dec;
  }
}

bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
  auto* class_method = cls_.GetMethod(method);
  if (!class_method) {
    return false;
  }

  if (class_method->formal_params.size() != argument_count) {
    cerr << "There's a class method " << cls_.GetName() << "." << method
      << " but it has " << class_method->formal_params.size() << "instead of " << argument_count
      << endl;
    return false;
  }

  return true;
}

const Closure& ClassInstance::Fields() const {
  return fields_;
}

Closure& ClassInstance::Fields() {
  return fields_;
}

ClassInstance::ClassInstance(const Class& cls)
  : cls_(cls)
{
}

ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
  if (!HasMethod(method, actual_args.size())) {
    ostringstream error_message;
    error_message << "Method " << cls_.GetName() << "." << method
      << " which takes " << actual_args.size() << " argument(s)"
      << " doesn't exists";
    throw std::runtime_error(error_message.str());
  }

  auto& instance_method = *cls_.GetMethod(method);
  Closure argument_closure = {{"self", ObjectHolder::Share(*this)}};

  transform(
    begin(instance_method.formal_params), end(instance_method.formal_params),
    begin(actual_args),
    inserter(argument_closure, end(argument_closure)),
    [](string formal, ObjectHolder actual) { return pair{formal, actual}; }
  );

  return instance_method.body->Execute(argument_closure);
}

void Class::AddParentMethods(const Class* parent) {
  if (parent) {
    this->AddParentMethods(parent->parent_);
    for (const auto& method : parent->methods_impl_) {
      methods_[method.name] = &method;
    }
  }
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent)
  : name_(move(name))
  , parent_(parent)
{
  AddParentMethods(parent_);

  methods_impl_.reserve(methods.size() + 1);
  for (auto&& method : methods) {
    methods_impl_.push_back(move(method));
    methods_[methods_impl_.back().name] = &methods_impl_.back();
  }

  if (static const string init_method = "__init__"; !GetMethod(init_method)) {
    methods_impl_.push_back(Method{init_method, {}, make_unique<Ast::None>()});
    methods_[init_method] = &methods_impl_.back();
  }
}

const Method* Class::GetMethod(const std::string& name) const {
  return methods_.count(name) ? methods_.at(name) : nullptr;
}

void Class::Print(ostream& os) {
  throw std::runtime_error("not implemented!");
}

const std::string& Class::GetName() const {
  return name_;
}

void Bool::Print(std::ostream& os) {
    os << (GetValue() ? "True" : "False");
}

} /* namespace Runtime */
