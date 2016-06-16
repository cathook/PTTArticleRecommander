#include "logging/logger.h"

#include <stdlib.h>

#include <unistd.h>

using std::string;


namespace logging {


Logger::Logger(string const& name): name_(name) {}

Logger::~Logger() {}

Logger* Logger::CreateSubLogger(string const& name) const {
  return new Logger(name_.empty() ? name : name_ + "." + name);
}

void Logger::Info(string const& text) {
  printf("[*] %s: %s\n", name_.c_str(), text.c_str());
}

void Logger::Warn(string const& text) {
  printf("[-] %s: %s\n", name_.c_str(), text.c_str());
}

void Logger::Fatal(string const& text) {
  printf("[X] %s: %s\n", name_.c_str(), text.c_str());
  exit(1);
}


namespace {


Logger* root_logger_;

}  // namespace


Logger* GetRootLogger() {
  if (root_logger_ == NULL) {
    root_logger_ = new Logger("");
  }
  return root_logger_;
}

}  // namespace logging
