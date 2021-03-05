#pragma once

#include <cstddef>
#include <utility>
#include <memory>

template <typename T>
struct RawMemory {
  T* raw_ = nullptr;
  size_t capacity_ = 0;

  RawMemory() = default;
  RawMemory(size_t capacity);
  RawMemory(const RawMemory<T>&) = delete;
  RawMemory(RawMemory<T>&& other);
  ~RawMemory();

  void Swap(RawMemory<T>& other);

  RawMemory<T>& operator=(const RawMemory<T>& other) = delete;
  RawMemory<T>& operator=(RawMemory<T>&& other);

  const T& operator[](size_t idx) const;
  T& operator[](size_t idx);

  const T* operator+(size_t idx) const;
  T* operator+(size_t idx);

  static T* Allocate(size_t count);
  static void Free(T* data);
};

template <typename T>
T* RawMemory<T>::Allocate(size_t count) {
  return static_cast<T*>(operator new(count * sizeof(T)));
}

template <typename T>
void RawMemory<T>::Free(T* data) {
  operator delete(data);
}

template <typename T>
RawMemory<T>::RawMemory(size_t capacity)
  : raw_(Allocate(capacity))
  , capacity_(capacity)
{
}

template <typename T>
RawMemory<T>::RawMemory(RawMemory<T>&& other) {
  Swap(other);
}

template <typename T>
RawMemory<T>::~RawMemory()
{
  Free(raw_);
}

template <typename T>
void RawMemory<T>::Swap(RawMemory<T>& other) {
  std::swap(raw_, other.raw_);
  std::swap(capacity_, other.capacity_);
}

template <typename T>
RawMemory<T>& RawMemory<T>::operator=(RawMemory<T>&& other) {
  Swap(other);
  return *this;
}

template <typename T>
const T& RawMemory<T>::operator[](size_t idx) const {
  return raw_[idx];
}

template <typename T>
T& RawMemory<T>::operator[](size_t idx) {
  return raw_[idx];
}

template <typename T>
const T* RawMemory<T>::operator+(size_t idx) const {
  return raw_ + idx;
}

template <typename T>
T* RawMemory<T>::operator+(size_t idx) {
  return raw_ + idx;
}

template <typename T>
class Vector {
public:
  Vector() = default;
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

private:
  RawMemory<T> data_;
  size_t size_ = 0;

  void Swap(Vector<T>& other);
};

template <typename T>
Vector<T>::Vector(size_t n)
  : data_(n)
{
  std::uninitialized_value_construct_n(data_.raw_, n);
  size_ = n;
}

template <typename T>
Vector<T>::Vector(const Vector& other)
  : data_(other.size_)
{
  std::uninitialized_copy_n(other.data_.raw_, other.size_, data_.raw_);
  size_ = other.size_;
}

template <typename T>
Vector<T>::Vector(Vector&& other) {
  Swap(other);
}

template <typename T>
Vector<T>::~Vector() {
  std::destroy_n(data_.raw_, size_);
}

template <typename T>
Vector<T>& Vector<T>::operator = (const Vector<T>& other) {
  if (other.size_ > data_.capacity_) {
    Vector<T> tmp(other);
    Swap(tmp);
  } else {
    for (size_t i = 0; i < size_ && i < other.size_; ++i) {
      data_[i] = other[i];
    }
    if (size_ < other.size_) {
      std::uninitialized_copy_n(other.data_.raw_ + size_, other.size_ - size_, data_.raw_ + size_);
    } else if (size_ > other.size_) {
      std::destroy_n(data_.raw_ + other.size_, size_ - other.size_);
    }
    size_ = other.size_;
  }
  return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator = (Vector<T>&& other) noexcept {
  Swap(other);
  return *this;
}

template <typename T>
void Vector<T>::Reserve(size_t n) {
  if (n > data_.capacity_) {
    RawMemory<T> new_data(n);
    std::uninitialized_move_n(data_.raw_, size_, new_data.raw_);
    std::destroy_n(data_.raw_, size_);
    data_.Swap(new_data);
  }
}

template <typename T>
void Vector<T>::Resize(size_t n) {
  Reserve(n);
  if (size_ < n) {
    std::uninitialized_value_construct_n(data_ + size_, n - size_);
  } else if (size_ > n) {
    std::destroy_n(data_ + n, size_ - n);
  }
  size_ = n;
}

template <typename T>
void Vector<T>::PushBack(const T& elem) {
  if (size_ == data_.capacity_) {
    Reserve(size_ == 0 ? 1 : 2 * size_);
  }
  new (data_ + size_) T(elem);
  ++size_;
}

template <typename T>
void Vector<T>::PushBack(T&& elem) {
  if (size_ == data_.capacity_) {
    Reserve(size_ == 0 ? 1 : 2 * size_);
  }
  new (data_ + size_) T(std::move(elem));
  ++size_;
}

template <typename T>
template <typename ... Args>
T& Vector<T>::EmplaceBack(Args&&... args) {
  if (size_ == data_.capacity_) {
    Reserve(size_ == 0 ? 1 : 2 * size_);
  }
  new (data_ + size_) T(std::forward<Args>(args)...);
  ++size_;
  return data_[size_ - 1];
}

template <typename T>
void Vector<T>::PopBack() {
  std::destroy_at(data_ + size_ - 1);
  --size_;
}

template <typename T>
size_t Vector<T>::Size() const noexcept {
  return size_;
}

template <typename T>
size_t Vector<T>::Capacity() const noexcept {
  return data_.capacity_;
}

template <typename T>
const T& Vector<T>::operator[](size_t i) const {
  return data_[i];
}

template <typename T>
T& Vector<T>::operator[](size_t i) {
  return data_[i];
}

template <typename T>
void Vector<T>::Swap(Vector<T>& other) {
  data_.Swap(other.data_);
  std::swap(size_, other.size_);
}

