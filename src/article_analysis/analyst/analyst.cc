#include "analyst.h"

#include <string>

#include "impl_/empty_analyst.h"
#include "impl_/fake_analyst.h"
#include "impl_/tf_idf_analyst_content.h"
#include "impl_/tf_idf_analyst_title.h"

using std::string;


namespace analyst {


Options::Options() : AOptionCollection("Options for the analyst.") {
  AddOption<utils::TypedOption<std::string>>(
      "impl_type", "tf_idf_title",
      "Specify the analysis algorithm to use. Acceptable options are"
      " tf_idf_title, tf_idf_content, empty and fake."
      " (default=tf_idf_title)");

  AddOption<impl_::FakeAnalystOptions>("fake_opts");
}


IAnalyst* CreateAnalyst(Options const& options,
                        miner::Miner* miner, logging::Logger* parent_logger) {
  string impl_type =
      options.GetOption<utils::TypedOption<string>>("impl_type")->value();

  if (impl_type == "empty") {
    return new impl_::EmptyAnalyst();
  }
  if (impl_type == "tf_idf_title") {
    return new impl_::TfIdfAnalystTitle(miner, parent_logger);
  }
  if (impl_type == "tf_idf_content") {
    return new impl_::TfIdfAnalystContent(miner, parent_logger);
  }
  if (impl_type == "fake") {
    return new impl_::FakeAnalyst(
        miner, *options.GetOption<impl_::FakeAnalystOptions>("fake_opts"));
  }
  
  logging::Logger* logger = parent_logger->CreateSubLogger("AnalystCreater");
  logger->Fatal("Unknown impl_type: %s", impl_type);
  delete logger;
  return NULL;
}

}  // namespace analyst
