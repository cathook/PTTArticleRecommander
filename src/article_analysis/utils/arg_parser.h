#ifndef ARTICLE_ANALYSIS_UTILS_ARG_PARSER_H_
#define ARTICLE_ANALYSIS_UTILS_ARG_PARSER_H_


#include <functional>
#include <string>
#include <unordered_map>
#include <vector>


namespace utils {


/**
 * A class for parsing the `argc, argv` arguments.
 */
class ArgParser {
 public:
  typedef std::function<bool()> OnFlagHandler;
  typedef std::function<bool(bool)> FlagHandler;
  typedef std::function<bool(std::string const&)> OptHandler;

  ArgParser();

  ~ArgParser();

  /**
   * Adds a flag.
   *
   * @param [in] name Name of the flag.
   * @param [in] desc Description of this flag.
   * @param [in] handler A callback function which will be called if
   *     the flag is set.
   */
  void AddFlag(std::string const& name, std::string const& desc,
               OnFlagHandler const& handler);

  /**
   * Adds a flag.
   *
   * @param [in] name Name of the flag.
   * @param [in] desc Description of this flag.
   * @param [in] handler A callback function which will be called with
   *     a boolean parameter, `true` if the flag is set.  Note that
   *     this callback function will always be called in the `Parse()`
   *     method.
   */
  void AddFlag(std::string const& name, std::string const& desc,
               FlagHandler const& handler);

  /**
   * Adds an option.
   *
   * @param [in] name Name of the option.
   * @param [in] value_name Name of the value, this is used while generating
   * @param [in] desc Description of this option.
   *     the usage information.
   * @param [in] handler A callback function which will be called with a
   *     parameter being the option value.
   */
  void AddOpt(std::string const& name, std::string const& value_name,
              std::string const& desc,
              OptHandler const& handler);

  /**
   * Adds an optional option.
   *
   * @param [in] name Name of the option.
   * @param [in] value_name Name of the value, this is used while generating
   * @param [in] desc Description of this option.
   *     the usage information.
   * @param [in] handler A callback function which will be called if a value
   *     is specified.
   * @param [in] default_value Default value of this option.
   * @param [in] call_handler_if_not_set Whether the method `Parse()` should
   *     call the `handler(default_value)` or not if the value is not set.
   */
  void AddOptionalOpt(std::string const& name, std::string const& value_name,
                      std::string const& desc,
                      OptHandler const& handler,
                      std::string const& default_value,
                      bool call_handler_if_not_set=true);

  /**
   * Parses the `argc, argv` pair.
   *
   * It will removes the parsed flags and options from the argument list.
   *
   * @param [in,out] argc Number of arguments.
   * @param [in,out] argv Arguments.
   * @return `true` if no any error happened while parsing.
   */
  bool Parse(int* argc, char const** argv);

  /**
   * @return The error message of the error got while parsing.
   */
  std::string const& err_msg() const;

  /**
   * Returns the usage string of the program.
   *
   * @return Usage.
   */
  std::string GetArgsUsage() const;

  /**
   * Returns the argument descriptions string of the program.
   *
   * @return Arguments description.
   */
  std::string GetArgsDesc(size_t indent=0) const;

 private:
  struct ArgInfo_ {
    std::string name;
    std::string usage;
    std::string desc;

    ArgInfo_(std::string const& name,
             std::string const& usage,
             std::string const& desc) : name(name), usage(usage), desc(desc) {}
  };

  struct OptionalOptInfo_ {
    OptHandler handler;
    std::string default_value;

    OptionalOptInfo_(OptHandler const& handler,
                     std::string const& default_value) :
        handler(handler), default_value(default_value) {}
  };

  void EnsureNameIsOkey_(std::string const& name) const;

  std::unordered_map<std::string, FlagHandler> flag_handlers_;
  std::unordered_map<std::string, OptHandler> opt_handlers_;
  std::unordered_map<std::string, OptionalOptInfo_> optional_opt_infos_;
  std::vector<ArgInfo_> arg_infos_;
  std::string err_msg_;
};

}  // namespace utils

#endif  // ARTICLE_ANALYSIS_UTILS_ARG_PARSER_H_
