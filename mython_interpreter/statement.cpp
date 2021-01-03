#include "statement.h"
#include "object.h"
#include "object_holder.h"

#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <numeric>
#include <stdexcept>

using namespace std;

namespace Ast {

using Runtime::Closure;

ObjectHolder Assignment::Execute(Closure& closure) {
  return closure[var_name] = right_value->Execute(closure);
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv)
  : var_name(move(var))
  , right_value(move(rv))
{
}

VariableValue::VariableValue(std::string var_name)
  : dotted_ids({var_name})
{
}

VariableValue::VariableValue(std::vector<std::string> dotted_ids)
  : dotted_ids(move(dotted_ids))
{
}

ObjectHolder VariableValue::Execute(Closure& closure) {
  const auto& var_name = dotted_ids.front();
  if (!closure.count(var_name)) {
    throw std::runtime_error("unknown variable " + var_name);
  }

  return accumulate(
    next(begin(dotted_ids)), end(dotted_ids),
    closure.at(var_name),
    [](ObjectHolder parent, const string& name) { return parent.TryAs<Runtime::ClassInstance>()->Fields().at(name); }
  );
}

unique_ptr<Print> Print::Variable(std::string var) {
  return make_unique<Print>(make_unique<VariableValue>(var));
}

Print::Print(unique_ptr<Statement> argument)
{
  args.push_back(move(argument));
}

Print::Print(vector<unique_ptr<Statement>> args)
  : args(move(args))
{
}

ObjectHolder Print::Execute(Closure& closure) {
  bool first{true};
  for (auto& argument : args) {
    if (first) {
      first = false;
    } else {
      *output << ' ';
    }

    if (auto object = argument->Execute(closure).Get(); object) {
      object->Print(*output);
    } else {
      *output << "None";
    }
  }
  *output << '\n';
  return ObjectHolder::None();
}

ostream* Print::output = &cout;

void Print::SetOutputStream(ostream& output_stream) {
  output = &output_stream;
}

MethodCall::MethodCall(
  std::unique_ptr<Statement> object
  , std::string method
  , std::vector<std::unique_ptr<Statement>> args)
  : object_(move(object))
  , method_(move(method))
  , args_(move(args))
{
}

ObjectHolder MethodCall::Execute(Closure& closure) {
  auto executed_object = object_->Execute(closure);
  auto class_instance = executed_object.TryAs<Runtime::ClassInstance>();
  if (!class_instance) {
    throw std::runtime_error("cannot run method of not class instance");
  }

  vector<ObjectHolder> actual_args;
  transform(
    begin(args_), end(args_),
    back_inserter(actual_args),
    [&closure](auto& argument) { return argument->Execute(closure); }
  );

  return class_instance->Call(method_, actual_args);
}

ObjectHolder Stringify::Execute(Closure& closure) {
  ostringstream out;
  argument->Execute(closure)->Print(out);
  return ObjectHolder::Own(Runtime::String{out.str()});
}

ObjectHolder Add::Execute(Closure& closure) {
  auto lhs_res = lhs->Execute(closure);
  auto rhs_res = rhs->Execute(closure);

  {
    auto lhs_numeric = lhs_res.TryAs<Runtime::Number>();
    auto rhs_numeric = rhs_res.TryAs<Runtime::Number>();
    if (lhs_numeric && rhs_numeric) {
      return ObjectHolder::Own(Runtime::Number{lhs_numeric->GetValue() + rhs_numeric->GetValue()});
    }
  }

  {
    auto lhs_string = lhs_res.TryAs<Runtime::String>();
    auto rhs_string = rhs_res.TryAs<Runtime::String>();
    if (lhs_string && rhs_string) {
      return ObjectHolder::Own(Runtime::String{lhs_string->GetValue() + rhs_string->GetValue()});
    }
  }

  if (auto lhs_class_instance = lhs_res.TryAs<Runtime::ClassInstance>(); lhs_class_instance) {
    return lhs_class_instance->Call("__add__", {move(rhs_res)});
  }

  throw std::runtime_error("Wrong types for add operation");
}

ObjectHolder Sub::Execute(Closure& closure) {
  auto lhs_res = lhs->Execute(closure);
  auto rhs_res = rhs->Execute(closure);

  {
    auto lhs_numeric = lhs_res.TryAs<Runtime::Number>();
    auto rhs_numeric = rhs_res.TryAs<Runtime::Number>();
    if (lhs_numeric && rhs_numeric) {
      return ObjectHolder::Own(Runtime::Number{lhs_numeric->GetValue() - rhs_numeric->GetValue()});
    }
  }

  if (auto lhs_class_instance = lhs_res.TryAs<Runtime::ClassInstance>(); lhs_class_instance) {
    return lhs_class_instance->Call("__sub__", {move(rhs_res)});
  }

  throw std::runtime_error("Wrong types for sub operation");
}

ObjectHolder Mult::Execute(Runtime::Closure& closure) {
  auto lhs_res = lhs->Execute(closure);
  auto rhs_res = rhs->Execute(closure);

  {
    auto lhs_numeric = lhs_res.TryAs<Runtime::Number>();
    auto rhs_numeric = rhs_res.TryAs<Runtime::Number>();
    if (lhs_numeric && rhs_numeric) {
      return ObjectHolder::Own(Runtime::Number{lhs_numeric->GetValue() * rhs_numeric->GetValue()});
    }
  }

  if (auto lhs_class_instance = lhs_res.TryAs<Runtime::ClassInstance>(); lhs_class_instance) {
    return lhs_class_instance->Call("__mult__", {move(rhs_res)});
  }

  throw std::runtime_error("Wrong types for mult operation");
}

ObjectHolder Div::Execute(Runtime::Closure& closure) {
  auto lhs_res = lhs->Execute(closure);
  auto rhs_res = rhs->Execute(closure);

  {
    auto lhs_numeric = lhs_res.TryAs<Runtime::Number>();
    auto rhs_numeric = rhs_res.TryAs<Runtime::Number>();

    if (lhs_numeric && rhs_numeric) {
      if (rhs_numeric->GetValue() == 0) {
        throw std::invalid_argument("division by zero");
      }
      return ObjectHolder::Own(Runtime::Number{lhs_numeric->GetValue() / rhs_numeric->GetValue()});
    }
  }

  if (auto lhs_class_instance = lhs_res.TryAs<Runtime::ClassInstance>(); lhs_class_instance) {
    return lhs_class_instance->Call("__div__", {move(rhs_res)});
  }

  throw std::runtime_error("Wrong types for div operation");
}

ObjectHolder Compound::Execute(Closure& closure) {
  for (auto& statement : statements) {
    auto ret = statement->Execute(closure);

    bool check_for_return = false
      || dynamic_cast<Return*>(statement.get())
      || dynamic_cast<IfElse*>(statement.get())
      || dynamic_cast<Compound*>(statement.get());

    if (check_for_return && ret.Get()) {
      return ret;
    }
  }
  return ObjectHolder::None();
;
}

ObjectHolder Return::Execute(Closure& closure) {
  return statement->Execute(closure);
}

ClassDefinition::ClassDefinition(ObjectHolder class_)
  : cls(move(class_))
  , class_name(cls.TryAs<Runtime::Class>()->GetName())
{
}

ObjectHolder ClassDefinition::Execute(Runtime::Closure& closure) {
  if (closure.count(class_name)) {
    throw std::runtime_error("redefinition of " + class_name);
  }
  closure[class_name] = cls;
  return ObjectHolder::None();
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
  auto instance = accumulate(
    next(begin(object.dotted_ids)), end(object.dotted_ids),
    closure.at(object.dotted_ids.front()).TryAs<Runtime::ClassInstance>(),
    [](Runtime::ClassInstance* parent, const string& id) { return parent->Fields().at(id).TryAs<Runtime::ClassInstance>(); }
  );

  return instance->Fields()[field_name] = right_value->Execute(closure);
}

IfElse::IfElse(
  std::unique_ptr<Statement> condition,
  std::unique_ptr<Statement> if_body,
  std::unique_ptr<Statement> else_body
)
  : condition(move(condition))
  , if_body(move(if_body))
  , else_body(move(else_body))
{
}

ObjectHolder IfElse::Execute(Runtime::Closure& closure) {
  if (Runtime::IsTrue(condition->Execute(closure))) {
    return if_body->Execute(closure);
  } else if (else_body) {
    return else_body->Execute(closure);
  }
  return ObjectHolder::None();
}

ObjectHolder Or::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(
    Runtime::Bool(Runtime::IsTrue(lhs->Execute(closure)) || Runtime::IsTrue(rhs->Execute(closure)))
  );
}

ObjectHolder And::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(
    Runtime::Bool(Runtime::IsTrue(lhs->Execute(closure)) && Runtime::IsTrue(rhs->Execute(closure)))
  );
}

ObjectHolder Not::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(
    Runtime::Bool(!Runtime::IsTrue(argument->Execute(closure)))
  );
}

Comparison::Comparison(
  Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs
)
  : comparator(move(cmp))
  , left(move(lhs))
  , right(move(rhs))
{
}

ObjectHolder Comparison::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(
    Runtime::Bool(comparator(left->Execute(closure), right->Execute(closure)))
  );
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
  vector<ObjectHolder> init_args;

  transform(
    begin(args), end(args),
    back_inserter(init_args),
    [&closure](unique_ptr<Statement>& statement) { return statement->Execute(closure); }
  );

  auto instance = ObjectHolder::Own(Runtime::ClassInstance{class_});
  instance.TryAs<Runtime::ClassInstance>()->Call("__init__", init_args);
  return instance;
}


} /* namespace Ast */
