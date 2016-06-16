#ifndef ARTICLE_ANALYSIS_ANALYST_I_ANALYST_H_
#define ARTICLE_ANALYSIS_ANALYST_I_ANALYST_H_


#include <vector>

#include "analyst/doc_info.h"


namespace analyst {


class IAnalyst {
 public:
  virtual ~IAnalyst() {}

  virtual DocInfo GetDocInfo(DocIdentity const& doc_id) const = 0;

 protected:
  IAnalyst() {}
};

}  // namespace analyst

#endif  // ARTICLE_ANALYSIS_ANALYST_I_ANALYST_H_
