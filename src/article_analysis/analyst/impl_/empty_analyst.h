#ifndef ARTUCLE_ANALYSIS_ANALYST_IMPL__EMPTY_ANALYST_H_
#define ARTUCLE_ANALYSIS_ANALYST_IMPL__EMPTY_ANALYST_H_


#include "analyst/i_analyst.h"


namespace analyst {

namespace impl_ {


/**
 * Just an fool analyst, it will return nothing.
 */
class EmptyAnalyst : public IAnalyst {
 public:
  DocRelInfo GetDocRelInfo(DocIdentity const& id) const override final {
    return DocRelInfo();
  }
};

}  // namespace impl_

}  // namespace analyst

#endif  // ARTUCLE_ANALYSIS_ANALYST_IMPL__EMPTY_ANALYST_H_
