#ifndef ARTICLE_ANALYSIS_UTILS_VARIANT_H_
#define ARTICLE_ANALYSIS_UTILS_VARIANT_H_


#include <memory>
#include <type_traits>
#include <typeindex>
#include <utility>

#include "utils/types.h"


namespace utils {


class Variant;


namespace variant_internal_ {


template <typename Type>
inline Type* GetAddress(Variant* self);

template <typename Type>
inline Type const* GetAddress(Variant const* self);

}  // namespace variant_internal_


/**
 * A container which can store variant type of value.
 *
 * The value type is no need to be desided at the compile time.
 */
class Variant {
 public:
  Variant() : ptr_(nullptr), type_(typeid(void)) {}

  Variant(Variant const& rhs) : ptr_(rhs.ptr_->Clone()), type_(rhs.type_) {}

  Variant(Variant&& rhs) : ptr_(std::move(rhs.ptr_)), type_(rhs.type_) {}

  ~Variant() {}

  /**
   * Checks whether this container contains nothing or not.
   *
   * @return `true` if it contains nothing.
   */
  bool IsEmpty() const { return !ptr_; }

  /**
   * Checks whether this container contains the specific type value.
   *
   * Note that if the container contains nothing, the type will be considered
   * as `void`.
   *
   * @return `true` if the type of the value stored is `Type`.
   */
  template <typename Type>
  bool IsTypeOf() const { return type_ == typeid(Type); }

  /**
   * Clears up this container.
   *
   * After calling this function, it will contains nothing.
   */
  void Clear() { Swap(Variant()); }

  /**
   * Copys the stored value from another container.
   *
   * @param [in] rhs The source value container.
   */
  void CopyFrom(Variant const& rhs) { Swap(Variant(rhs)); }

  void CopyFrom(Variant&& rhs) { Swap(std::forward<Variant>(rhs)); }

  /**
   * Swaps itself with another container.
   *
   * @param [in] rhs The container to swap with.
   */
  void Swap(Variant& rhs) {
    std::swap(ptr_, rhs.ptr_);
    std::swap(type_, rhs.type_);
  }

  void Swap(Variant&& rhs) { ptr_ = std::move(rhs.ptr_); type_ = rhs.type_; }

  /**
   * @return The stored value.
   */
  template <typename Type>
  Type& value() {
    assert(IsTypeOf<Type>());
    return dynamic_cast<TypePtr_<Type>*>(ptr_.get())->value;
  }

  template <typename Type>
  Type const& value() const {
    assert(IsTypeOf<Type>());
    return dynamic_cast<TypePtr_<Type> const*>(ptr_.get())->value;
  }

  /**
   * @return The address which the value placed at.
   */
  template <typename Type>
  Type* address() { return variant_internal_::GetAddress<Type>(this); }

  template <typename Type>
  Type const* address() const {
    return variant_internal_::GetAddress<Type>(this);
  }

  /**
   * Sets the value to be stored.
   *
   * @param [in] value The value to be stored.
   */
  template <typename Type>
  void set_value(Type const& value) {
    ptr_.reset(new TypePtr_<RemoveRef<Type>>(value));
    type_ = typeid(RemoveRef<Type>);
  }

  /**
   * @see CopyFrom()
   */
  Variant& operator=(Variant const& rhs) { CopyFrom(rhs); return *this; }

  Variant& operator=(Variant&& rhs) {
    CopyFrom(std::forward<Variant>(rhs));
    return *this;
  }

 private:
  template <typename Type>
  friend Type* variant_internal_::GetAddress(Variant*);

  template <typename Type>
  friend Type const* variant_internal_::GetAddress(Variant const*);

  struct PtrBase_ {
    virtual ~PtrBase_() {}
    virtual PtrBase_* Clone() const = 0;

    virtual void* void_address() = 0;
    virtual void const* void_address() const = 0;
  };

  template <typename Type>
  struct TypePtr_ : PtrBase_ {
    TypePtr_(Type const& value) : value(value) {}
    TypePtr_(Type&& value) : value(std::forward<Type>(value)) {}

    PtrBase_* Clone() const override final { return new TypePtr_<Type>(value); }

    void* void_address() override final {
      return reinterpret_cast<void*>(&value);
    }

    void const* void_address() const override final {
      return reinterpret_cast<void const*>(&value);
    }

    Type value;
  };

  std::unique_ptr<PtrBase_> ptr_;
  std::type_index type_;
};


namespace variant_internal_ {


template <typename Type>
inline Type* GetAddress(Variant* self) {
  return &self->value<Type>();
}

template <typename Type>
inline Type const* GetAddress(Variant const* self) {
  return &self->value<Type>();
}

template <>
inline void* GetAddress<void>(Variant* self) {
  return self->ptr_->void_address();
}

template <>
inline void const* GetAddress<void>(Variant const* self) {
  return self->ptr_->void_address();
}

}  // namespace variant_internal_

}  // namespace utils

#endif  // ARTICLE_ANALYSIS_UTILS_VARIANT_H_
