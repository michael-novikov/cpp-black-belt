#include "vector.h"

#include "test_runner.h"

using namespace std;

class C {
public:
  inline static int created = 0;
  inline static int assigned = 0;
  inline static int deleted = 0;
  static void Reset() {
    created = assigned = deleted = 0;
  }

  C() {
    ++created;
  }
  C(int x) {
    ++created;
  }
  C(const C& other) {
    ++created;
  }
  C& operator=(const C& other) {
    ++assigned;
    return *this;
  }
  ~C() {
    ++deleted;
  }
};

void TestInit() {
  {
    C::Reset();
    Vector<C> v(3);
    ASSERT(C::created == 3 && C::assigned == 0 && C::deleted == 0);
  }
  ASSERT(C::deleted == 3);
};


void TestAssign() {
  {
    C::Reset();
    Vector<C> v1(2), v2(3);
    ASSERT(C::created == 5 && C::assigned == 0 && C::deleted == 0);
    v1 = v2;
    ASSERT(C::created == 8 && C::assigned == 0 && C::deleted == 2);
    ASSERT(v1.Size() == 3 && v2.Size() == 3);
  }
  ASSERT(C::deleted == 8);

  {
    C::Reset();
    Vector<C> v1(3), v2(2);
    ASSERT(C::created == 5 && C::assigned == 0 && C::deleted == 0);
    v1 = v2;
    ASSERT(C::created == 5 && C::assigned == 2 && C::deleted == 1);
    ASSERT(v1.Size() == 2 && v2.Size() == 2);
  }
  ASSERT(C::deleted == 5);
}

void TestPushBack() {
  {
    C::Reset();
    Vector<C> v;
    C c;
    v.PushBack(c);
    ASSERT(C::created == 2 && C::assigned == 0 && C::deleted == 0);

    v.PushBack(c);  // reallocation
    ASSERT(C::created == 4 && C::assigned == 0 && C::deleted == 1);
  }
  ASSERT(C::deleted == 4);
}

void TestEmplaceBack() {
  {
    C::Reset();
    Vector<C> v;
    v.EmplaceBack(42);
    ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 0);

    v.EmplaceBack(41);  // reallocation
    ASSERT(C::created == 3 && C::assigned == 0 && C::deleted == 1);
  }
  ASSERT(C::deleted == 3);
}

void TestCopyAssignment() {
  {
    C c;
    C::Reset();

    Vector<C> v1;
    v1.Reserve(10);
    v1.PushBack(c);

    ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 0);

    Vector<C> v2;
    v2.PushBack(c);
    v2.PushBack(c);

    ASSERT(C::created == 4 && C::assigned == 0 && C::deleted == 1);

    v1 = v2;

    ASSERT(C::created == 5 && C::assigned == 1 && C::deleted == 1);
  }
  {
    C c;
    C::Reset();

    Vector<C> v1;
    v1.Reserve(10);
    v1.PushBack(c);

    ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 0);

    Vector<C> v2;
    v2.Reserve(10);
    v2.PushBack(c);
    v2.PushBack(c);

    ASSERT(C::created == 3 && C::assigned == 0 && C::deleted == 0);

    C::Reset();

    v2 = v1;

    ASSERT(C::created == 0 && C::assigned == 1 && C::deleted == 1);
  }
  {
    C c;
    C::Reset();

    Vector<C> v1;
    v1.Reserve(10);

    ASSERT(C::created == 0 && C::assigned == 0 && C::deleted == 0);

    Vector<C> v2;
    v2.PushBack(c);
    v2.PushBack(c);

    ASSERT(C::created == 3 && C::assigned == 0 && C::deleted == 1);

    C::Reset();

    v2 = v1;

    ASSERT(C::created == 0 && C::assigned == 0 && C::deleted == 2);
  }
  {
    C c;
    C::Reset();

    Vector<C> v1;
    v1.Reserve(999);
    v1.PushBack(c);

    ASSERT(C::created == 1 && C::assigned == 0 && C::deleted == 0);

    C::Reset();

    Vector<C> v2;
    v2.PushBack(c);
    v2.PushBack(c);

    ASSERT(C::created == 3 && C::assigned == 0 && C::deleted == 1);

    C::Reset();

    v2 = v1;

    ASSERT(C::created == 0 && C::assigned == 1 && C::deleted == 1);
  }
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestInit);
  RUN_TEST(tr, TestAssign);
  RUN_TEST(tr, TestPushBack);
  RUN_TEST(tr, TestEmplaceBack);
  RUN_TEST(tr, TestCopyAssignment);
  return 0;
}
