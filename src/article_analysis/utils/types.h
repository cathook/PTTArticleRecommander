#ifndef ARTICLE_ANALYSIS_UTILS_TYPES_H_
#define ARTICLE_ANALYSIS_UTILS_TYPES_H_


#include <sstream>
#include <string>
#include <type_traits>


namespace utils {


template <typename T>
using RemoveRef = typename std::remove_reference<T>::type;

}  // namespace utils

#endif  // ARTICLE_ANALYSIS_UTILS_TYPES_H_
