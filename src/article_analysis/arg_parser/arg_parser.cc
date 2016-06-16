#include "arg_parser/arg_parser.h"

#include <assert.h>
#include <string.h>

#include <string>
#include <unordered_set>
#include <vector>

#include "utils/funcs.h"



using std::string;
using std::unordered_set;
using std::vector;


namespace arg_parser {


ArgParser::ArgParser() {}

ArgParser::~ArgParser() {}

void ArgParser::AddFlag(string const& name, string const& desc,
                        OnFlagHandler const& handler) {
  AddFlag(name, desc,
          [handler](bool value) { return value ? handler() : true; });
}

void ArgParser::AddFlag(string const& name, string const& desc,
                        FlagHandler const& handler) {
  EnsureNameIsOkey_(name);

  flag_handlers_.emplace(name, handler);

  arg_infos_.emplace_back(name, "[" + name + "]", desc);
}

void ArgParser::AddOpt(string const& name, string const& value_name,
                       string const& desc,
                       OptHandler const& handler) {
  EnsureNameIsOkey_(name);

  opt_handlers_.emplace(name, handler);

  auto real_name = utils::FormatStr("%s <%s>", name, value_name);
  arg_infos_.emplace_back(real_name, real_name, desc);
}

void ArgParser::AddOptionalOpt(string const& name, string const& value_name,
                               string const& desc,
                               OptHandler const& handler,
                               string const& default_value,
                               bool call_handler_if_not_set) {
  if (!call_handler_if_not_set) {
    auto real_handler = [default_value, handler](std::string const& value) {
      return  (value != default_value ? handler(value) : true);
    };
    AddOptionalOpt(name, value_name, desc, real_handler, default_value, true);
    return;
  }

  EnsureNameIsOkey_(name);

  optional_opt_infos_.emplace(name, OptionalOptInfo_(handler, default_value));

  auto real_name = utils::FormatStr("%s <%s>", name, value_name);
  auto real_desc = utils::FormatStr("(optional, default=`%s`) %s",
                                    default_value, desc);
  arg_infos_.emplace_back(real_name, "[" + real_name + "]", real_desc);
}

bool ArgParser::Parse(int* argc, char const** argv) {
  // TODO(b01902109<at>ntu.edu.tw): Refactor this fat method.
  vector<char const*> unknown;
  unordered_set<string> handled_args;

  for (int i = 1; i < *argc; ++i) {
    if (strcmp(argv[i], "--") == 0) {
      if (i + 1 >= *argc) {
        err_msg_ = "The notation `--` should be followed by an argument.";
        return false;
      }
      unknown.push_back(argv[++i]);
      continue;
    }
    {
      auto it = flag_handlers_.find(argv[i]);
      if (it != flag_handlers_.end()) {
        if (!it->second(true)) {
          err_msg_ = utils::FormatStr("Invalid flag `%s`.", it->first);
          return false;
        }
        handled_args.emplace(argv[i]);
        continue;
      }
    }
    {
      auto it = opt_handlers_.find(argv[i]);
      if (it != opt_handlers_.end()) {
        if (i + 1 >= *argc) {
          err_msg_ = utils::FormatStr(
              "The option `%s` should be followed by an argument.", it->first);
          return false;
        }
        if (!it->second(argv[i + 1])) {
          err_msg_ = utils::FormatStr(
              "Invalid option value `%s=%s`.", it->first, argv[i + 1]);
          return false;
        }
        handled_args.emplace(argv[i]);
        ++i;
        continue;
      }
    }
    {
      auto it = optional_opt_infos_.find(argv[i]);
      if (it != optional_opt_infos_.end()) {
        if (i + 1 >= *argc) {
          err_msg_ = utils::FormatStr(
              "The option `%s` should be followed by an argument.", it->first);
          return false;
        }
        if (!it->second.handler(argv[i + 1])) {
          err_msg_ = utils::FormatStr(
              "Invalid option value `%s=%s`.", it->first, argv[i + 1]);
          return false;
        }
        handled_args.emplace(argv[i]);
        ++i;
        continue;
      }
    }
    unknown.push_back(argv[i]);
  }
  for (auto it : flag_handlers_) {
    if (handled_args.count(it.first) == 0) {
      if (!it.second(false)) {
        err_msg_ = utils::FormatStr("Invalid flag `%s`.", it.first);
        return false;
      }
    }
  }
  for (auto it : opt_handlers_) {
    if (handled_args.count(it.first) == 0) {
      err_msg_ = utils::FormatStr("The option `%s` is not optional.", it.first);
      return false;
    }
  }
  for (auto it : optional_opt_infos_) {
    if (handled_args.count(it.first) == 0) {
      if (!it.second.handler(it.second.default_value)) {
        err_msg_ = utils::FormatStr(
            "Invalid option `%s=%s`.", it.first, it.second.default_value);
        return false;
      }
    }
  }
  *argc = unknown.size() + 1;
  for (int i = 0, i_end = unknown.size(); i < i_end; ++i) {
    argv[1 + i] = unknown[i];
  }
  return true;
}

string const& ArgParser::err_msg() const {
  return err_msg_;
}

string ArgParser::GetArgsUsage() const {
  string splitter("");
  string ret;

  for (auto& it : arg_infos_) {
    ret += splitter + it.usage;
    splitter = " ";
  }

  return ret;
}

string ArgParser::GetArgsDesc(size_t indent) const {
  string indent_str(indent, ' ');
  string ret;

  for (auto& it : arg_infos_) {
    ret += indent_str + it.name + "\t" + it.desc + "\n";
  }

  return ret;
}

void ArgParser::EnsureNameIsOkey_(string const& name) const {
  assert(!name.empty());
  assert(name != "--");
  assert(flag_handlers_.count(name) == 0);
  assert(opt_handlers_.count(name) == 0);
  assert(optional_opt_infos_.count(name) == 0);
}

}  // namespace arg_parser
