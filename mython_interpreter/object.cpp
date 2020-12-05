#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>

using namespace std;

namespace Runtime {

void ClassInstance::Print(std::ostream& os) {
}

bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
}

const Closure& ClassInstance::Fields() const {
}

Closure& ClassInstance::Fields() {
}

ClassInstance::ClassInstance(const Class& cls) {
}

ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent) {
}

const Method* Class::GetMethod(const std::string& name) const {
}

void Class::Print(ostream& os) {
}

const std::string& Class::GetName() const {
}

void Bool::Print(std::ostream& os) {
}

} /* namespace Runtime */
