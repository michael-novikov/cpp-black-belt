#include "statement.h"
#include "object.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace Ast {

using Runtime::Closure;

ObjectHolder Assignment::Execute(Closure& closure) {
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) {
}

VariableValue::VariableValue(std::string var_name) {
}

VariableValue::VariableValue(std::vector<std::string> dotted_ids) {
}

ObjectHolder VariableValue::Execute(Closure& closure) {
}

unique_ptr<Print> Print::Variable(std::string var) {
}

Print::Print(unique_ptr<Statement> argument) {
}

Print::Print(vector<unique_ptr<Statement>> args) {
}

ObjectHolder Print::Execute(Closure& closure) {
}

ostream* Print::output = &cout;

void Print::SetOutputStream(ostream& output_stream) {
  output = &output_stream;
}

MethodCall::MethodCall(
  std::unique_ptr<Statement> object
  , std::string method
  , std::vector<std::unique_ptr<Statement>> args
)
{
}

ObjectHolder MethodCall::Execute(Closure& closure) {
}

ObjectHolder Stringify::Execute(Closure& closure) {
}

ObjectHolder Add::Execute(Closure& closure) {
}

ObjectHolder Sub::Execute(Closure& closure) {
}

ObjectHolder Mult::Execute(Runtime::Closure& closure) {
}

ObjectHolder Div::Execute(Runtime::Closure& closure) {
}

ObjectHolder Compound::Execute(Closure& closure) {
}

ObjectHolder Return::Execute(Closure& closure) {
}

ClassDefinition::ClassDefinition(ObjectHolder class_) {
}

ObjectHolder ClassDefinition::Execute(Runtime::Closure& closure) {
}

FieldAssignment::FieldAssignment(
  VariableValue object, std::string field_name, std::unique_ptr<Statement> rv
)
  : object(std::move(object))
  , field_name(std::move(field_name))
  , right_value(std::move(rv))
{
}

ObjectHolder FieldAssignment::Execute(Runtime::Closure& closure) {
}

IfElse::IfElse(
  std::unique_ptr<Statement> condition,
  std::unique_ptr<Statement> if_body,
  std::unique_ptr<Statement> else_body
)
{
}

ObjectHolder IfElse::Execute(Runtime::Closure& closure) {
}

ObjectHolder Or::Execute(Runtime::Closure& closure) {
}

ObjectHolder And::Execute(Runtime::Closure& closure) {
}

ObjectHolder Not::Execute(Runtime::Closure& closure) {
}

Comparison::Comparison(
  Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs
) {
}

ObjectHolder Comparison::Execute(Runtime::Closure& closure) {
}

NewInstance::NewInstance(
  const Runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args
)
  : class_(class_)
  , args(std::move(args))
{
}

NewInstance::NewInstance(const Runtime::Class& class_) : NewInstance(class_, {}) {
}

ObjectHolder NewInstance::Execute(Runtime::Closure& closure) {
}


} /* namespace Ast */
