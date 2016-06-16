#include "protocol/net.h"

#include <locale.h>


namespace protocol {

namespace net {

namespace internal_ {

namespace {


void Init_();


void Pass_();


void (*init_func_)() = Init_;


void Init_() {
  setlocale(LC_ALL, "");
  init_func_ = Pass_;
}


void Pass_() {}

}  // namespace


void InitIfFirst() { init_func_(); }

}  // namespace internal_

}  // namespace net

}  // namespace protocol
