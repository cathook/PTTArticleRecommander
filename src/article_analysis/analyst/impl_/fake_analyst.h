#ifndef ARTUCLE_ANALYSIS_ANALYST_IMPL__FAKE_ANALYST_H_
#define ARTUCLE_ANALYSIS_ANALYST_IMPL__FAKE_ANALYST_H_


#include "analyst/i_analyst.h"
#include "miner/miner.h"
#include "utils/options.h"


namespace analyst {

namespace impl_ {


class FakeAnalystOptions : public utils::AOptionCollection {
 public:
  FakeAnalystOptions() : utils::AOptionCollection("") {
    AddOption<utils::TypedOption<std::string>>("board_name", "Gossiping", "");
  }
};


class FakeAnalyst : public IAnalyst {
 public:
  FakeAnalyst(miner::Miner* miner, FakeAnalystOptions const& opt);

  DocRelInfo GetDocRelInfo(DocIdentity const& id) const override final {
    return DocRelInfo();
  }
};

}  // namespace impl_

}  // namespace analyst

#endif  // ARTUCLE_ANALYSIS_ANALYST_IMPL__FAKE_ANALYST_H_

