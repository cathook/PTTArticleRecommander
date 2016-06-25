#ifndef ARTICLE_ANALYSIS_ANALYST_IMPL__TF_IDF_ANALYST_TITLE_H_
#define ARTICLE_ANALYSIS_ANALYST_IMPL__TF_IDF_ANALYST_TITLE_H_


#include <vector>

#include "analyst/i_analyst.h"
#include "logging/logger.h"
#include "miner/miner.h"


namespace analyst {

namespace impl_ {


class TfIdfAnalystTitle : public IAnalyst {
 public:
  TfIdfAnalystTitle(miner::Miner* miner, logging::Logger* parent_logger);

  ~TfIdfAnalystTitle();

  DocRelInfo GetDocRelInfo(DocIdentity const& id) const override final;

 private:
  static char const* const SKIP_CHARS_;

  void RunMainAlgo_(miner::Miner* miner);

  static void SanitizeContent_(std::string* s);

  logging::Logger* logger_;
  int id_base_;
  std::vector<std::vector<int> > recommended_file_;
  std::vector<bool> file_calculated_;
};

}  // namespace impl_

}  // namespace analyst

#endif  // ARTICLE_ANALYSIS_ANALYST_IMPL__TF_IDF_ANALYST_TITLE_H_
