#include "utils/funcs.h"

#include <string.h>

#include <string>

#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "logging/logger.h"


using std::string;


namespace utils {

namespace {


string* SetAndGetPackageRoot_();

string* GetPackageRoot_();


string* package_root_ = NULL;

string* (*get_package_root_func_)() = SetAndGetPackageRoot_;



string* SetAndGetPackageRoot_() {
  static char dest[PATH_MAX + 1];
  int ret = readlink(FormatStr("/proc/%d/exe", getpid()).c_str(),
                     dest, PATH_MAX + 1);
  if (ret == -1) {
    logging::GetRootLogger()->Fatal("Cannot readlink(): %s", strerror(errno));
  }
  package_root_ = new string(dirname(dirname(dest)));
  get_package_root_func_ = GetPackageRoot_;
  return get_package_root_func_();
}

string* GetPackageRoot_() { return package_root_; }

}  // namespace


string const& GetPackageRoot() { return *get_package_root_func_(); }

}  // namespace utils
