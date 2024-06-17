//
// Created by Алексей on 15.06.2024.
//

#define WEAK_PTR_IMPLEMENTED
#ifndef UNIQUE_PTR_H
#define UNIQUE_PTR_H
#include <iostream>
class BadWeakPtr : public std::exception {
  const char* what() const noexcept override {
    return "BadWeakPtr";
  }
};

struct Counter {
  int strong_count;
  int weak_count;
  Counter() : strong_count(1), weak_count(0) {
  }
};

template <typename T>
class WeakPtr;

template <class T>
class SharedPtr {
  friend class WeakPtr<T>;

 private:
  T* shared_ptr_ = nullptr;
  Counter* counter_ = nullptr;

 public:
  void Delete();
  SharedPtr();
  SharedPtr(T* pointer);  // NOLINT
  SharedPtr(const SharedPtr&);
  SharedPtr(SharedPtr&&) noexcept;
  SharedPtr(const WeakPtr<T>&);  // NOLINT
  ~SharedPtr();

  SharedPtr& operator=(const SharedPtr&);
  SharedPtr& operator=(SharedPtr&&) noexcept;
  SharedPtr& operator=(T* ptr);

  void Reset(T* ptr = nullptr);
  void Swap(SharedPtr&);

  T* Get() const;
  int UseCount() const;
  T& operator*() const;
  T* operator->() const;
  explicit operator bool() const;
};

template <class T>
SharedPtr<T>::SharedPtr() : shared_ptr_(nullptr), counter_(nullptr) {
}

template <class T>
SharedPtr<T>::SharedPtr(T* pointer) : shared_ptr_(pointer) {
  if (pointer == nullptr) {
    counter_ = nullptr;
  } else {
    counter_ = new Counter();
  }
}

template <class T>
SharedPtr<T>::SharedPtr(const SharedPtr& ptr) : shared_ptr_(ptr.shared_ptr_), counter_(ptr.counter_) {
  if (counter_ != nullptr) {
    counter_->strong_count += 1;
  }
}

template <class T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& ptr) {
  if (this->shared_ptr_ == ptr.shared_ptr_) {
    return *this;
  }
  Delete();
  shared_ptr_ = ptr.shared_ptr_;
  counter_ = ptr.counter_;
  if (counter_ != nullptr) {
    counter_->strong_count += 1;
  }
  return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(T* ptr) {
  Delete();
  shared_ptr_ = ptr;
  if (ptr != nullptr) {
    counter_->strong_count = 1;
    counter_->weak_count = 0;
  } else {
    counter_ = nullptr;
  }
  return *this;
}

template <class T>
SharedPtr<T>::SharedPtr(const WeakPtr<T>& ptr) : shared_ptr_(ptr.weak_ptr_), counter_(ptr.counter_) {
  if (ptr.Expired()) {
    throw BadWeakPtr{};
  }
  counter_->strong_count += 1;
}

template <class T>
SharedPtr<T>::SharedPtr(SharedPtr&& ptr) noexcept : shared_ptr_(ptr.shared_ptr_), counter_(ptr.counter_) {
  ptr.shared_ptr_ = nullptr;
  ptr.counter_ = nullptr;
}

template <class T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& ptr) noexcept {
  if (this == &ptr) {
    return *this;
  }
  if (this->shared_ptr_ == ptr.shared_ptr_) {
    counter_->strong_count -= 1;
  } else {
    Delete();
    shared_ptr_ = ptr.shared_ptr_;
    counter_ = ptr.counter_;
  }
  ptr.shared_ptr_ = nullptr;
  ptr.counter_ = nullptr;
  return *this;
}

template <class T>
SharedPtr<T>::~SharedPtr() {
  Delete();
}

template <class T>
void SharedPtr<T>::Reset(T* ptr) {
  Delete();
  shared_ptr_ = ptr;
  if (ptr != nullptr) {
    counter_ = new Counter();
  } else {
    counter_ = nullptr;
  }
}

template <class T>
void SharedPtr<T>::Swap(SharedPtr& ptr) {
  std::swap(counter_, ptr.counter_);
  std::swap(shared_ptr_, ptr.shared_ptr_);
}

template <class T>
T* SharedPtr<T>::Get() const {
  return shared_ptr_;
}

template <class T>
int SharedPtr<T>::UseCount() const {
  if (counter_ == nullptr) {
    return 0;
  }
  return counter_->strong_count;
}

template <class T>
T& SharedPtr<T>::operator*() const {
  return *shared_ptr_;
}

template <class T>
T* SharedPtr<T>::operator->() const {
  return shared_ptr_;
}

template <class T>
SharedPtr<T>::operator bool() const {
  return shared_ptr_ != nullptr;
}

template <class T>
void SharedPtr<T>::Delete() {
  if (counter_ == nullptr) {
    return;
  }
  counter_->strong_count -= 1;
  if (counter_->strong_count == 0) {
    delete shared_ptr_;
    if (counter_->weak_count == 0) {
      delete counter_;
    }
  }
}

template <class T>
class WeakPtr {
  friend class SharedPtr<T>;

 private:
  T* weak_ptr_ = nullptr;
  Counter* counter_ = nullptr;

 public:
  void Delete();

  WeakPtr();
  WeakPtr(const WeakPtr&);
  WeakPtr(WeakPtr&&) noexcept;
  WeakPtr(const SharedPtr<T>&);  // NOLINT
  ~WeakPtr();

  WeakPtr& operator=(const WeakPtr&);
  WeakPtr& operator=(WeakPtr&&) noexcept;

  void Swap(WeakPtr&);
  void Reset();
  int UseCount() const;
  bool Expired() const;
  SharedPtr<T> Lock() const;
};

template <class T>
WeakPtr<T>::WeakPtr() : weak_ptr_(nullptr), counter_(nullptr) {
}

template <class T>
WeakPtr<T>::WeakPtr(const WeakPtr& ptr) : weak_ptr_(ptr.weak_ptr_), counter_(ptr.counter_) {
  if (counter_ != nullptr) {
    counter_->weak_count += 1;
  }
}

template <class T>
WeakPtr<T>::WeakPtr(WeakPtr&& ptr) noexcept : weak_ptr_(ptr.weak_ptr_), counter_(ptr.counter_) {
  ptr.weak_ptr_ = nullptr;
  ptr.counter_ = nullptr;
}

template <class T>
WeakPtr<T>::~WeakPtr() {
  Delete();
}
template <class T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& ptr) {
  if (this->weak_ptr_ == ptr.weak_ptr_) {
    return *this;
  }
  Delete();
  weak_ptr_ = ptr.weak_ptr_;
  counter_ = ptr.counter_;
  if (counter_ != nullptr) {
    counter_->weak_count += 1;
  }
  return *this;
}

template <class T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& ptr) noexcept {
  if (this == &ptr) {
    return *this;
  }
  if (this->weak_ptr_ == ptr.weak_ptr_) {
    counter_->weak_count -= 1;
  } else {
    Delete();
    weak_ptr_ = ptr.weak_ptr_;
    counter_ = ptr.counter_;
  }
  ptr.weak_ptr_ = nullptr;
  ptr.counter_ = nullptr;
  return *this;
}

template <class T>
void WeakPtr<T>::Delete() {
  if (counter_ == nullptr) {
    return;
  }
  counter_->weak_count -= 1;
  if (counter_->strong_count == 0 && counter_->weak_count == 0) {
    delete counter_;
  }
}

template <class T>
void WeakPtr<T>::Swap(WeakPtr& ptr) {
  std::swap(counter_, ptr.counter_);
  std::swap(weak_ptr_, ptr.weak_ptr_);
}

template <class T>
void WeakPtr<T>::Reset() {
  Delete();
  weak_ptr_ = nullptr;
  counter_ = nullptr;
}

template <class T>
int WeakPtr<T>::UseCount() const {
  if (counter_ == nullptr) {
    return 0;
  }
  return counter_->strong_count;
}

template <class T>
bool WeakPtr<T>::Expired() const {
  return (counter_ == nullptr || counter_->strong_count == 0);
}

template <typename T>
SharedPtr<T> WeakPtr<T>::Lock() const {
  if (this->Expired()) {
    return {};
  }
  return {*this};
}

template <class T>
WeakPtr<T>::WeakPtr(const SharedPtr<T>& ptr) : weak_ptr_(ptr.shared_ptr_), counter_(ptr.counter_) {
  if (counter_ == nullptr) {
    return;
    ;
  }
  counter_->weak_count += 1;
}

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
  return {new T(std::forward<Args>(args)...)};
}

#endif  // UNIQUE_PTR_H
