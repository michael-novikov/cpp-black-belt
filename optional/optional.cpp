#include "optional.h"

#include "test_runner.h"

using namespace std;

class C {
public:
  inline static int created = 0;
  inline static int assigned = 0;
  inline static int deleted = 0;
  inline static int moved = 0;
  static void Reset() {
    //created = assigned = deleted = 0;
    created = assigned = deleted = moved = 0;
  }

  C() {
    ++created;
  }
  C(const C& other) {
    ++created;
  }
  C(C&& other) {
    ++moved;
  }
  C& operator=(const C& other) {
    ++assigned;
    return *this;
  }
  C& operator=(C&& other) {
    ++moved;
    return *this;
  }
  ~C() {
    ++deleted;
  }
};

void TestInit() {
  {
    C::Reset();
    C c;
    Optional<C> o(c);
    ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 0);
  }
  ASSERT(C::deleted == 2);
};


void TestAssign() {
  Optional<C> o1, o2;

  { // Assign a Value to empty
    C::Reset();
    C c;
    o1 = c;
    ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 0);
  }
  { // Assign a non-empty to empty
    C::Reset();
    o2 = o1;
    ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 0);
  }
  { // Assign non-empty to non-empty
    C::Reset();
    o2 = o1;
    ASSERT(C::created == 0 && C::assigned == 1 && C::deleted == 0);
  }
  { // Assign empty to non-empty
    Optional<C> o3;
    C::Reset();
    o2 = o3;
    ASSERT(C::created == 0 && C::assigned == 0 && C::deleted == 1);
  }
}

void TestMove() {
  Optional<C> o1, o2;

  { // Assign a Value to empty
    C::Reset();
    C c;
    o1 = std::move(c);
    ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 0 && C::moved == 1);
  }
  { // Assign a non-empty to empty
    C::Reset();
    o2 = std::move(o1);
    ASSERT(C::created == 0 && C::assigned == 0 && C::deleted == 0 && C::moved == 1);
  }
  { // Assign non-empty to non-empty
    Optional<C> o3(std::move(C()));
    C::Reset();
    o2 = std::move(o3);
    ASSERT(C::created == 0 && C::assigned == 0 && C::deleted == 0 && C::moved == 1);
  }
  { // Assign empty to non-empty
    Optional<C> o3;
    C::Reset();
    o2 = std::move(o3);
    ASSERT(C::created == 0 && C::assigned == 0 && C::deleted == 1 && C::moved == 0);
  }
}

void TestReset() {
  C::Reset();
  Optional<C> o = C();
  o.Reset();
  //ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 2);
  ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 2 && C::moved == 1);
}

void TestHasValue() {
  Optional<int> o;
  ASSERT(!o.HasValue());

  o = 42;
  ASSERT(o.HasValue());

  o.Reset();
  ASSERT(!o.HasValue());
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestInit);
  RUN_TEST(tr, TestAssign);
  RUN_TEST(tr, TestMove);
  RUN_TEST(tr, TestReset);
  RUN_TEST(tr, TestHasValue);
  return 0;
}
