#ifndef ARTICLE_ANALYSIS_UTILS_I_RANGE_ITERATE_SUPPORT_H_
#define ARTICLE_ANALYSIS_UTILS_I_RANGE_ITERATE_SUPPORT_H_


namespace utils {


/**
 * An interface which support C++ range iterate syntax.
 */
template <typename IteratorType, typename ConstIteratorType>
class IRangeIterateSupport {
 public:
  virtual ~IRangeIterateSupport() {}

  virtual IteratorType begin() = 0;
  virtual IteratorType end() = 0;

  virtual ConstIteratorType begin() const = 0;
  virtual ConstIteratorType end() const = 0;

 protected:
  IRangeIterateSupport() {}
};

}  // namespace utils

#endif  // ARTICLE_ANALYSIS_UTILS_I_RANGE_ITERATE_SUPPORT_H_
