#ifndef ARTICLE_ANALYSIS_UTILS_OPTIONS_H_
#define ARTICLE_ANALYSIS_UTILS_OPTIONS_H_


#include <assert.h>
#include <ctype.h>
#include <stddef.h>

#include <iterator>
#include <memory>
#include <stack>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "utils/funcs.h"
#include "utils/i_range_iterate_support.h"
#include "utils/variant.h"


namespace utils {


/**
 * Base interface which all option class should implement it.
 */
class IOptionBase {
 public:
  virtual ~IOptionBase() {}

  /**
   * @return The description of this option.
   */
  virtual std::string const& desc() const = 0;

 protected:
  IOptionBase() {}
};


/**
 * A single option, it stores a value without name.
 */
class AAnOption : public IOptionBase {
 public:
  virtual ~AAnOption() {}

  std::string const& desc() const override final { return desc_; }

  /**
   * @return The stored option value.  Note that it assumes the caller
   *     knows the type of the value and specified the type by the template
   *     paramenter.
   */
  template <typename Type>
  Type const& value() const { return GetValue().value<Type>(); }

  /**
   * Sets the option value.
   *
   * It assumes the caller knows the type of the value this option container
   * supports and the template parameter must be compatable to that type.
   *
   * @param [in] value The value to be stored.
   */
  template <typename Type>
  void set_value(Type const& value) {
    Variant wrapper;
    wrapper.set_value<RemoveRef<Type>>(value);
    return SetValue(wrapper);
  }

 protected:
  AAnOption(std::string const& desc) : desc_(desc) {}

  virtual Variant const& GetValue() const = 0;

  virtual void SetValue(Variant const& value) = 0;

 private:
  std::string desc_;
};


class AOptionCollection : public IOptionBase {
 private:
  template <bool IGNORE_OPTION_COLLECTION>
  class Iterator_;

 public:
  typedef Iterator_<false> NormalIterator;

  typedef Iterator_<true> AnOptionOnlyIterator;

  class IteratorValueType : public IOptionBase {
   public:
    IteratorValueType() : IteratorValueType("", NULL) {}

    bool operator==(IteratorValueType const& rhs) const {
      return ptr_ == rhs.ptr_;
    }

    std::string const& name() const { return name_; }

    std::string const& desc() const override final { return ptr_->desc(); }

    IOptionBase const* address() const { return ptr_; }

   private:
    IteratorValueType(std::string const& name, IOptionBase const* ptr) :
        name_(name), ptr_(ptr) {}

    friend NormalIterator;
    friend AnOptionOnlyIterator;

    std::string name_;
    IOptionBase const* ptr_;
  };

 private:
  struct IteratorStackEntry_;

  template <typename IterType>
  class TypeIterators_;

  template <bool IGNORE_OPTION_COLLECTION>
  class Iterator_ {
   public:
    typedef ptrdiff_t difference_type;

    typedef IteratorValueType const value_type;

    typedef value_type& reference;

    typedef value_type* pointer;

    typedef std::input_iterator_tag iterator_category;

    Iterator_(Iterator_ const& rhs);

    Iterator_(Iterator_&& rhs);

    ~Iterator_();

    Iterator_& operator=(Iterator_ const& rhs);

    Iterator_& operator=(Iterator_&& rhs);

    bool operator==(Iterator_ const& rhs) const;

    bool operator!=(Iterator_ const& rhs) const;

    Iterator_& operator++();

    Iterator_ operator++(int) const;

    reference operator*() const;

    pointer operator->() const;

   private:
    Iterator_();

    Iterator_(AOptionCollection const* start_point);

    friend class TypeIterators_<Iterator_>;

    void GoNextInState_();

    bool GoNextStateIfNeed_();

    void SetupCurrValue_();

    std::stack<IteratorStackEntry_> stack_;
    IteratorValueType curr_value_;
  };

  template <typename IterType>
  class TypeIterators_ : public IRangeIterateSupport<IterType, IterType> {
   public:
    IterType begin() { return IterType(ptr_); }

    IterType end() { return end_; }

    IterType begin() const { return IterType(ptr_); }

    IterType end() const { return end_; }

   private:
    friend class AOptionCollection;

    TypeIterators_(AOptionCollection const* ptr) : ptr_(ptr), end_() {}

    AOptionCollection const* ptr_;
    IterType end_;
  };

 public:
  typedef TypeIterators_<NormalIterator> NormalIterators;

  typedef TypeIterators_<AnOptionOnlyIterator> AnOptionOnlyIterators;

  virtual ~AOptionCollection();

  std::string const& desc() const override final { return desc_; }

  /**
   * @return Iterators for interating throught all the sub-options.
   */
  NormalIterators const& iterators() const {
    return normal_iterators_;
  }

  /**
   * @return Iterators for interating throught all the `AnOption`'s instances.
   */
  AnOptionOnlyIterators const& an_option_only_iterators() const {
    return an_option_only_iterators_;
  }

  /**
   * Gets a sub-option by name.
   *
   * Again, this function need the caller specifies the right type.
   *
   * @param [in] name Name of the sub-option.
   * @return A pointer points to the result.
   */
  template <typename Type>
  Type* GetOption(std::string const& name) {
    return const_cast<Type*>(GetOption_<Type>(name, 0));
  }

  template <typename Type>
  Type const* GetOption(std::string const& name) const {
    return GetOption_<Type>(name, 0);
  }

 protected:
  AOptionCollection(std::string const& desc);

  /**
   * Adds a sub-option.
   *
   * @param [in] name Name of the sub-option.  It should match with the
   *     regular expression `[a-zA-Z0-9_][a-zA-Z0-9_]*`.
   * @param [in] args The arguments for the constructor of the sub-option.
   */
  template <typename Type, typename... Args>
  void AddOption(std::string const& name, Args&& ...args) {
    EnsureNameValid_(name);
    assert(an_options_.count(name) == 0 &&
           option_collections_.count(name) == 0);
    auto ptr = new Type(std::forward<Args>(args)...);
    if (std::is_base_of<AAnOption, Type>::value) {
      assert(!(std::is_base_of<AOptionCollection, Type>::value));
      an_options_.emplace(
          name, std::unique_ptr<AAnOption>(dynamic_cast<AAnOption*>(ptr)));
      return;
    }
    if (std::is_base_of<AOptionCollection, Type>::value) {
      option_collections_.emplace(
          name, std::unique_ptr<AOptionCollection>(
              dynamic_cast<AOptionCollection*>(ptr)));
      return;
    }
    assert(false);
  }

 private:
  typedef std::unordered_map<
      std::string, std::unique_ptr<AAnOption>> AnOptionMap_;

  typedef std::unordered_map<
      std::string, std::unique_ptr<AOptionCollection>> OptionCollectionMap_;

  template <typename Type>
  Type const* GetOption_(std::string const& name, size_t name_start_pos) const {
    size_t dot_pos = name.find('.', name_start_pos);
    if (dot_pos != name.npos) {
      auto it = option_collections_.find(name.substr(0, dot_pos));
      assert(it != option_collections_.end());
      return it->second->GetOption_<Type>(name, dot_pos + 1);
    }
    std::string n2 = name.substr(name_start_pos);

    if (std::is_base_of<AAnOption, Type>::value) {
      auto it = an_options_.find(n2);
      assert(it != an_options_.end());
      return dynamic_cast<Type const*>(it->second.get());
    }
    if (std::is_base_of<AOptionCollection, Type>::value) {
      auto it = option_collections_.find(n2);
      assert(it != option_collections_.end());
      return dynamic_cast<Type const*>(it->second.get());
    }
    assert(false);
    return NULL;
  }

  static void EnsureNameValid_(std::string const& name) {
    assert(!name.empty());
    for (auto c : name) {
      assert(isalnum(c) || c == '_');
    }
  }

  std::string desc_;

  AnOptionMap_ an_options_;
  OptionCollectionMap_ option_collections_;

  NormalIterators normal_iterators_;
  AnOptionOnlyIterators an_option_only_iterators_;
};


/**
 * An option which stores value with a specificed  type.
 *
 * Setting the value with a string is acceptable, it will use
 * `utils::TransformFromStr` to transform to the proper type.
 */
template <typename Type>
class TypedOption : public AAnOption {
 public:
  TypedOption(Type const& default_value, std::string const& desc) :
      AAnOption(desc) {
    set_value(default_value);
  }

  /**
   * @return The stored value.
   */
  Type const& value() const { return value_.value<Type>(); }


  /**
   * Re-write the stored value.
   *
   * @param [in] value Value to be stored.
   */
  void set_value(Type const& value) { value_.set_value(value); }

 private:
  Variant const& GetValue() const override final {
    return value_;
  }

  void SetValue(Variant const& value) override final {
    if (value.IsTypeOf<Type>()) {
      value_.set_value<Type>(value.value<Type>());
    } else if (value.IsTypeOf<std::string>()) {
      value_.set_value<Type>(
          TransformFromStr<Type>(value.value<std::string>()));
    } else {
      assert(false);
    }
  }

  Variant value_;
};


}  // namespace utils

#endif  // ARTICLE_ANALYSIS_UTILS_OPTIONS_H_
