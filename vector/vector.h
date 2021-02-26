#include <cstddef>
#include <utility>

template <typename T>
class RawMemory {
public:
  RawMemory() = default;
  RawMemory(size_t capacity);
  ~RawMemory();

  const T& operator[](size_t idx) const;
  T& operator[](size_t idx);

  const T* operator+(size_t idx) const;
  T* operator+(size_t idx);
private:
  T* data_ = nullptr;
  size_t capacity_;

  static T* Allocate(size_t count);
  static void Free(T* data);
};

template <typename T>
static T* Allocate(size_t count) {
  return static_cast<T*>(operator new(count * sizeof(T)));
}

template <typename T>
static void Free(T* data) {
  operator delete(data);
}

template <typename T>
RawMemory<T>::RawMemory(size_t capacity)
{
  data_ = Allocate(capacity);
  capacity_ = capacity;
}

template <typename T>
RawMemory<T>::~RawMemory()
{
  Free(data_);
}

template <typename T>
const T& RawMemory<T>::operator[](size_t idx) const {
  return data_[idx];
}

template <typename T>
T& RawMemory<T>::operator[](size_t idx) {
  return data_[idx];
}

template <typename T>
const T* RawMemory<T>::operator+(size_t idx) const {
  return data_ + idx;
}

template <typename T>
T* RawMemory<T>::operator+(size_t idx) {
  return data_ + idx;
}

template <typename T>
class Vector {
public:
  Vector();
  Vector(size_t n);
  Vector(const Vector& other);
  Vector(Vector&& other);

  ~Vector();

  Vector& operator = (const Vector& other);
  Vector& operator = (Vector&& other) noexcept;

  void Reserve(size_t n);

  void Resize(size_t n);

  void PushBack(const T& elem);
  void PushBack(T&& elem);

  template <typename ... Args>
  T& EmplaceBack(Args&&... args);

  void PopBack();

  size_t Size() const noexcept;

  size_t Capacity() const noexcept;

  const T& operator[](size_t i) const;
  T& operator[](size_t i);
};
