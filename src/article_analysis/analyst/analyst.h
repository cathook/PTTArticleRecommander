#ifndef ARTICLE_ANALYSIS_ANALYST_ANALYST_H_
#define ARTICLE_ANALYSIS_ANALYST_ANALYST_H_


#include <string>

#include "analyst/i_analyst.h"
#include "impl_/fake_analyst.h"
#include "logging/logger.h"
#include "miner/miner.h"
#include "utils/options.h"


namespace analyst {


class Options : public utils::AOptionCollection {
 public:
  Options() : AOptionCollection("Options for the analyst.") {
    AddOption<utils::TypedOption<std::string>>(
        "impl_type", "tf_idf_title",
		"Implement type, default=tf_idf_title. "
		"Option: tf_idf_title, tf_idf_content");

    AddOption<impl_::FakeAnalystOptions>("fake_opts");
  }
};


/**
 * Creates the instance of analyst by the `impl_type` value in the options.
 *
 * @param [in] options The options.
 * @param [in] miner The miner.
 * @param [in] parent_logger The parent logger.
 *
 * @return Instances of an analyst.
 */
IAnalyst* CreateAnalyst(Options const& options,
                        miner::Miner* miner, logging::Logger* parent_logger);

}  // namespace analyst

#endif  // ARTICLE_ANALYSIS_ANALYST_ANALYST_H_
