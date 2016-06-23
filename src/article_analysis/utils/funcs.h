#ifndef ARTICLE_ANALYSIS_UTILS_FUNCS_H_
#define ARTICLE_ANALYSIS_UTILS_FUNCS_H_


#include <assert.h>
#include <stdio.h>

#include <memory>
#include <sstream>
#include <string>

#include "utils/types.h"


namespace utils {

namespace funcs_internal_ {


template <typename CCType>
struct CasterToC {
  static CCType const& Cast(CCType const& value) { return value; }
  static CCType&& Cast(CCType&& value) { return std::forward<CCType>(value); }
};


template <typename CharType, typename... Args>
struct CasterToC<std::basic_string<CharType, Args...>> {
  typedef std::basic_string<CharType, Args...> CCType;

  static CharType const* Cast(CCType const& value) { return value.c_str(); }
  static CharType const* Cast(CCType&& value) { return value.c_str(); }
};


template <typename CharType, typename... Args>
struct CasterToC<std::basic_string<CharType, Args...> const> {
  typedef std::basic_string<CharType, Args...> const CCType;

  static CharType const* Cast(CCType const& value) { return value.c_str(); }
  static CharType const* Cast(CCType&& value) { return value.c_str(); }
};

}  // namespace funcs_internal_


/**
 * A C++ style alternate of `sprintf`.
 *
 * Instead of giving a buffer for placing the result, this function will
 * return a C++ string, which buffer size will be automatically setup
 * inside the function.
 *
 * The detail usage is similar to the C function `printf`, except that
 * this function also allows `std::basic_string<CharType>` type argument
 * (such as `std::string`), such argument will be transform to
 * `CharType const*` before forwarding to the original `snprintf` function
 * by calling the method `.c_str()`
 *
 * @param [in] fmt The format string.
 * @param [in] args The remain arguments.
 * @return The result string.
 */
template <typename... Args>
std::string FormatStr(char const* fmt, Args&& ...args) {
  using namespace funcs_internal_;

  int n = snprintf(NULL, 0, fmt, CasterToC<RemoveRef<Args>>::Cast(args)...);
  assert(n >= 0);

  std::unique_ptr<char[]> buf(new char[n + 1]);

  int m = snprintf(buf.get(), n + 1, fmt,
                   CasterToC<RemoveRef<Args>>::Cast(args)...);
  assert(n == m);

  return std::string(buf.get());
}


namespace funcs_internal_ {


template <typename Type>
struct TransformFromStrImpl {
  static Type Transform(std::string const& s) {
    std::istringstream is(s);
    Type ret;
    is >> ret;
    return ret;
  }
};


template <>
struct TransformFromStrImpl<std::string> {
  static std::string Transform(std::string const& s) {
    return s;
  }
};

}  // namespace funcs_internal_


template <typename Type>
inline Type TransformFromStr(std::string const& s) {
  return funcs_internal_::TransformFromStrImpl<Type>::Transform(s);
}


std::string const& GetPackageRoot();


inline std::string GetShareFileFullPath(std::string const& s) {
  return GetPackageRoot() + "/share/article_analysis/" + s;
}

}  // namespace utils

#endif  // ARTICLE_ANALYSIS_UTILS_FUNCS_H_
