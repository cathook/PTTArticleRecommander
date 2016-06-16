#ifndef ARTICLE_ANALYSIS_ANALYST_DOC_INFO_H_
#define ARTICLE_ANALYSIS_ANALYST_DOC_INFO_H_


#include <vector>

#include "protocol/types.h"


namespace analyst {


struct DocIdentity {
  protocol::types::Board board;
  protocol::types::Identity id;
};


/**
 * Information results from the analyst.
 */
struct DocInfo {
  /**
   * The positive relative document's identity.
   */
  std::vector<DocIdentity> pos_rel_docs;

  /**
   * The negative relative document's identity.
   */
  std::vector<DocIdentity> neg_rel_docs;

  /**
   * The neutral relative document's identity.
   */
  std::vector<DocIdentity> neutral_rel_docs;

  DocInfo() {}
};

}  // namespace analyst

#endif  // ARTICLE_ANALYSIS_ANALYST_DOC_INFO_H_
