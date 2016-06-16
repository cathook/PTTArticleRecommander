// TODO(cathook): Implements full functions.

#ifndef ARTICLE_ANALYSIS_LOGGING_LOGGER_H_
#define ARTICLE_ANALYSIS_LOGGING_LOGGER_H_

#include <string>

#include "utils/funcs.h"


namespace logging {


class Logger {
 public:
  ~Logger();

  /**
   * Creates a sub logger.
   *
   * @param [in] name Name of the sub-logger.  The full name of the created
   *     logger will be `<curr_name>.<name>`.
   *
   * @return An instance of `Logger`.
   */
  Logger* CreateSubLogger(std::string const& name) const;

  /**
   * Prings information to the logger.
   *
   * @param [in] text The message to be printed.
   */
  void Info(std::string const& text);

  /**
   * Prings warning to the logger.
   *
   * @param [in] text The message to be printed.
   */
  void Warn(std::string const& text);

  /**
   * Prings fatal error to the logger and exits.
   *
   * @param [in] text The message to be printed.
   */
  void Fatal(std::string const& text);

  /**
   * @see Info(text)
   * printf arguments style.
   */
  template <typename Arg0, typename... Args>
  void Info(char const* fmt, Arg0 const& arg0, Args&&... args) {
    Info(utils::FormatStr(fmt, arg0, std::forward<Args>(args)...));
  }

  /**
   * @see Info(text)
   * printf arguments style.
   */
  template <typename Arg0, typename... Args>
  inline void Warn(char const* fmt, Arg0 const& arg0, Args&&... args) {
    Warn(utils::FormatStr(fmt, arg0, std::forward<Args>(args)...));
  }

  /**
   * @see Info(text)
   * printf arguments style.
   */
  template <typename Arg0, typename... Args>
  inline void Fatal(char const* fmt, Arg0 const& arg0, Args&&... args) {
    Fatal(utils::FormatStr(fmt, arg0, std::forward<Args>(args)...));
  }

  Logger(Logger const& rhs) = delete;

  Logger& operator=(Logger const& rhs) = delete;

 private:
  friend Logger* GetRootLogger();

  Logger(std::string const& name);

  std::string name_;
};


Logger* GetRootLogger();

}  // namespace logging

#endif  // ARTICLE_ANALYSIS_LOGGING_LOGGER_H_
