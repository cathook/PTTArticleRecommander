#ifndef ARTICLE_ANALYSIS_ANALYST_I_ANALYST_H_
#define ARTICLE_ANALYSIS_ANALYST_I_ANALYST_H_


#include <vector>

#include "protocol/types.h"


namespace analyst {


typedef protocol::types::DocRelInfo DocRelInfo;


typedef protocol::types::DocIdentity DocIdentity;


class IAnalyst {
 public:
  virtual ~IAnalyst() {}

  virtual DocRelInfo GetDocRelInfo(DocIdentity const& doc_id) const = 0;

 protected:
  IAnalyst() {}
};

}  // namespace analyst

#endif  // ARTICLE_ANALYSIS_ANALYST_I_ANALYST_H_
