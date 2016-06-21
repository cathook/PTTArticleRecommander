#include "analyst.h"

#include <string>

#include "impl_/empty_analyst.h"

#include "impl_/tf_idf_analyst.h"

using std::string;


namespace analyst {


IAnalyst* CreateAnalyst(Options const& options,
                        miner::Miner* miner, logging::Logger* parent_logger) {
  string impl_type =
      options.GetOption<utils::TypedOption<string>>("impl_type")->value();

  if (impl_type == "empty") {
    return new impl_::EmptyAnalyst();
  }
  else if (impl_type == "tf_idf") {
    return new impl_::TfIdfAnalyst(miner);
  }
  
  logging::Logger* logger = parent_logger->CreateSubLogger("AnalystCreater");
  logger->Fatal("Unknown impl_type: %s", impl_type);
  delete logger;
  return NULL;
}

}  // namespace analyst
