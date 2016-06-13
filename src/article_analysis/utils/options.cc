#include "utils/options.h"

#include <utility>


using std::move;
using std::string;


namespace utils {

namespace {


inline string ConcatStr_(string const& s, string const& t) {
  return s.empty() ? t : s + "." + t;
}

}  // namespace


struct AOptionCollection::IteratorStackEntry_ {
  enum class State : int {
    ITSELF,
    ITER_AN_OPTIONS,
    ITER_OPTION_COLLECTIONS
  };

  AOptionCollection const* root;
  string name;
  State state;

  AnOptionMap_::const_iterator an_option_it;
  AnOptionMap_::const_iterator an_option_it_end;
  OptionCollectionMap_::const_iterator option_collection_it;
  OptionCollectionMap_::const_iterator option_collection_it_end;

  IteratorStackEntry_(AOptionCollection const* root, string const& name) :
      root(root), name(name), state(State::ITSELF) {}
};


template <bool X>
AOptionCollection::Iterator_<X>::Iterator_() :
    stack_(), curr_value_("", NULL) {}

template <bool X>
AOptionCollection::Iterator_<X>::Iterator_(
    AOptionCollection const* start_point) {
  stack_.emplace(start_point, "");
  operator++();
}

template <bool X>
AOptionCollection::Iterator_<X>::Iterator_(Iterator_ const& rhs) :
    stack_(rhs.stack_), curr_value_(rhs.curr_value_) {}

template <bool X>
AOptionCollection::Iterator_<X>::Iterator_(Iterator_&& rhs) :
    stack_(move(rhs.stack_)), curr_value_(move(rhs.curr_value_)) {}

template <bool X>
AOptionCollection::Iterator_<X>::~Iterator_() {}

template <bool X>
AOptionCollection::Iterator_<X>& AOptionCollection::Iterator_<X>::operator=(
    Iterator_ const& rhs) {
  stack_ = rhs.stack_;
  curr_value_ = rhs.curr_value_;
  return *this;
}

template <bool X>
AOptionCollection::Iterator_<X>& AOptionCollection::Iterator_<X>::operator=(
    Iterator_&& rhs) {
  stack_ = move(rhs.stack_);
  curr_value_ = move(rhs.curr_value_);
  return *this;
}

template <bool X>
bool AOptionCollection::Iterator_<X>::operator==(Iterator_ const& rhs) const {
  return curr_value_ == rhs.curr_value_;
}

template <bool X>
bool AOptionCollection::Iterator_<X>::operator!=(Iterator_ const& rhs) const {
  return !operator==(rhs);
}

template <bool X>
AOptionCollection::Iterator_<X>& AOptionCollection::Iterator_<X>::operator++() {
  GoNextInState_();
  while (GoNextStateIfNeed_()) {}
  SetupCurrValue_();
  return *this;
}

template <bool X>
AOptionCollection::Iterator_<X>
AOptionCollection::Iterator_<X>::operator++(int) const {
  return ++Iterator_(*this);
}

template <bool X>
typename AOptionCollection::Iterator_<X>::reference
AOptionCollection::Iterator_<X>::operator*() const {
  return curr_value_;
}

template <bool X>
typename AOptionCollection::Iterator_<X>::pointer
AOptionCollection::Iterator_<X>::operator->() const {
  return &curr_value_;
}

template <bool X>
void AOptionCollection::Iterator_<X>::GoNextInState_() {
  auto& info = stack_.top();
  switch (info.state) {
  case IteratorStackEntry_::State::ITER_AN_OPTIONS:
    ++info.an_option_it;
    break;
  case IteratorStackEntry_::State::ITER_OPTION_COLLECTIONS:
    ++info.option_collection_it;
    break;
  default:
    break;
  }
}

template <bool IGNORE_OPTION_COLLECTION>
bool
AOptionCollection::Iterator_<IGNORE_OPTION_COLLECTION>::GoNextStateIfNeed_() {
  auto& info = stack_.top();

  if (info.state == IteratorStackEntry_::State::ITSELF) {
    info.state = IteratorStackEntry_::State::ITER_AN_OPTIONS;
    info.an_option_it = info.root->an_options_.begin();
    info.an_option_it_end = info.root->an_options_.end();
    return true;
  }

  if (info.state == IteratorStackEntry_::State::ITER_AN_OPTIONS &&
      info.an_option_it == info.an_option_it_end) {
    info.state = IteratorStackEntry_::State::ITER_OPTION_COLLECTIONS;
    info.option_collection_it = info.root->option_collections_.begin();
    info.option_collection_it_end = info.root->option_collections_.end();
    return true;
  }

  if (info.state == IteratorStackEntry_::State::ITER_OPTION_COLLECTIONS) {
    if (info.option_collection_it != info.option_collection_it_end) {
      stack_.emplace(info.option_collection_it->second.get(),
                     ConcatStr_(info.name, info.option_collection_it->first));
      return IGNORE_OPTION_COLLECTION;
    } else {
      stack_.pop();
      if (!stack_.empty()) {
        ++stack_.top().option_collection_it;
        return true;
      }
    }
  }
  return false;
}

template <bool X>
void AOptionCollection::Iterator_<X>::SetupCurrValue_() {
  if (stack_.empty()) {
    curr_value_ = IteratorValueType();
  } else {
    auto& info = stack_.top();
    switch (info.state) {
    case IteratorStackEntry_::State::ITSELF:
      curr_value_ = IteratorValueType(info.name, info.root);
      break;
    case IteratorStackEntry_::State::ITER_AN_OPTIONS:
      curr_value_ = IteratorValueType(
          ConcatStr_(info.name, info.an_option_it->first),
          info.an_option_it->second.get());
      break;
    case IteratorStackEntry_::State::ITER_OPTION_COLLECTIONS:
      curr_value_ = IteratorValueType(
          ConcatStr_(info.name, info.option_collection_it->first),
          info.option_collection_it->second.get());
      break;
    }
  }
}


template class AOptionCollection::Iterator_<true>;
template class AOptionCollection::Iterator_<false>;


AOptionCollection::AOptionCollection(string const& desc) :
    desc_(desc),
    an_options_(), option_collections_(),
    normal_iterators_(this), an_option_only_iterators_(this) {}

AOptionCollection::~AOptionCollection() {}

}  // namespace utils
