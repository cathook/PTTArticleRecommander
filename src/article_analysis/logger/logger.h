// TODO(b01902109<at>ntu.edu.tw): Implements full functions.

#ifndef ARTICLE_ANALYSIS_LOGGER_LOGGER_H_
#define ARTICLE_ANALYSIS_LOGGER_LOGGER_H_

#include <string>
#include <utility>

#include "utils/funcs.h"


namespace logger {


inline void Info(std::string const& text) {
  printf("[*] %s\n", text.c_str());
}

template <typename Arg0, typename... Args>
inline void Info(char const* fmt, Arg0 const& arg0, Args&&... args) {
  Info(utils::FormatStr(fmt, arg0, std::forward<Args>(args)...));
}


inline void Warn(std::string const& text) {
  printf("[-] %s\n", text.c_str());
}

template <typename Arg0, typename... Args>
inline void Warn(char const* fmt, Arg0 const& arg0, Args&&... args) {
  Warn(utils::FormatStr(fmt, arg0, std::forward<Args>(args)...));
}


inline void Fatal(std::string const& text) {
  printf("[X] %s\n", text.c_str());
  exit(1);
}

template <typename Arg0, typename... Args>
inline void Fatal(char const* fmt, Arg0 const& arg0, Args&&... args) {
  Fatal(utils::FormatStr(fmt, arg0, std::forward<Args>(args)...));
}


}  // namespace logger

#endif  // ARTICLE_ANALYSIS_LOGGER_LOGGER_H_
