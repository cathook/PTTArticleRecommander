// TODO(cathook): Refactor this ugly code.

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <unordered_set>

#include "arg_parser/arg_parser.h"
#include "logging/logger.h"
#include "miner/miner.h"
#include "utils/funcs.h"
#include "utils/options.h"


using std::string;
using std::unordered_set;


namespace {


class Options : public utils::AOptionCollection {
 public:
  Options() : AOptionCollection("") {
    AddOption<miner::Options>("miner");
  }
};


struct Configs {
};


Options* options;
Configs* configs;


void SetOptionValue(string const& name, string const& value) {
  options->GetOption<utils::AAnOption>(name)->set_value<string>(value);
}


void HandleProgArgs(int argc, char** argv) {
  arg_parser::ArgParser ap;

  bool need_help = false;
  ap.AddFlag("-h", "Prints the help document.",
             [&need_help]() { need_help = true; return true; });

  unordered_set<string> option_names;
  for (auto it : options->an_option_only_iterators()) {
    option_names.insert(it.name());
  }
  auto handler = [&option_names](string const& s) {
    size_t eq_pos = s.find('=');
    if (eq_pos == s.npos) {
      return false;
    }
    string name = s.substr(0, eq_pos);
    string value = s.substr(eq_pos + 1);
    if (option_names.count(name) == 0) {
      return false;
    }
    SetOptionValue(name, value);
    return true;
  };
  ap.AddOptionalOpt("-o", "option",
                    "Sets an option, see next section for detail informations",
                    handler, "", false);

  bool succ = ap.Parse(&argc, const_cast<char const**>(argv));

  if (need_help) {
    string doc;
    doc += "[USAGE]\n";
    doc += utils::FormatStr("    %s %s\n", argv[0], ap.GetArgsUsage());
    doc += "\n";
    doc += "[DESCRIPTION]\n";
    doc += "    A simple article analysis server.  Program parameters:\n";
    doc += ap.GetArgsDesc(4);
    doc += "\n";
    doc += "[AVAILIABLE OPTIONS]\n";
    for (auto it : options->iterators()) {
      doc += utils::FormatStr("    %s\t%s\n", it.name(), it.desc());
    }
    printf("%s\n", doc.c_str());
    exit(0);
  }

  if (!succ) {
    fprintf(stderr, "Got an error while parsing the arguments: `%s`.\n",
            ap.err_msg().c_str());
    fprintf(stderr, "Use flag `-h` for detail usage.\n");
    exit(1);
  }
}

}  // namespace


int main(int argc, char** argv) {
  options = new Options();
  configs = new Configs();

  HandleProgArgs(argc, argv);

  miner::Miner* m = new miner::Miner(
      *options->GetOption<miner::Options>("miner"), logging::GetRootLogger());

  delete m;
  return 0;
}
