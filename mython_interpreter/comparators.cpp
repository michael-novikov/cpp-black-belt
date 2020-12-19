#include "comparators.h"
#include "object.h"
#include "object_holder.h"

#include <functional>
#include <optional>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace Runtime {

bool Equal(ObjectHolder lhs, ObjectHolder rhs) {
  if (lhs.TryAs<String>() && rhs.TryAs<String>()) {
    return lhs.TryAs<String>()->GetValue() == rhs.TryAs<String>()->GetValue();
  } else if (lhs.TryAs<Number>() && rhs.TryAs<Number>()) {
    return lhs.TryAs<Number>()->GetValue() == rhs.TryAs<Number>()->GetValue();
  }

  throw std::runtime_error("unsupported operand types for Equal()");
}

bool Less(ObjectHolder lhs, ObjectHolder rhs) {
  if (lhs.TryAs<String>() && rhs.TryAs<String>()) {
    return lhs.TryAs<String>()->GetValue() < rhs.TryAs<String>()->GetValue();
  } else if (lhs.TryAs<Number>() && rhs.TryAs<Number>()) {
    return lhs.TryAs<Number>()->GetValue() < rhs.TryAs<Number>()->GetValue();
  }

  throw std::runtime_error("unsupported operand types for Less()");
}

} /* namespace Runtime */
