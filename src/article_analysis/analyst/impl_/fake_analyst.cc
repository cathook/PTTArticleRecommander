#include "analyst/impl_/fake_analyst.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string>

#include "utils/options.h"


using std::string;


namespace analyst {

namespace impl_ {


FakeAnalyst::FakeAnalyst(miner::Miner* miner, FakeAnalystOptions const& opt) {
  string board_name =
      opt.GetOption<utils::TypedOption<string>>("board_name")->value();
  auto max_id = miner->GetMaxId(board_name);
  printf("max_id = %d\n", (int)max_id);

  while (true) {
    printf(">>> ");
    fflush(stdout);
    static char buf[10];
    if (scanf("%2s", buf) != 1) {
      break;
    }
    if (buf[0] == '?') {
      max_id = miner->GetMaxId(board_name);
      printf("max_id = %d\n", (int)max_id);
      continue;
    }
    int k;
    if (scanf("%d", &k) != 1) {
      break;
    }
    if (buf[0] == 'r') {
      auto r = miner->GetDocRealData(board_name, k);
      printf("%s\n", r.content.c_str());
      for (auto& rr : r.reply_messages) {
        printf("r: %s\n", rr.message.c_str());
      }
      continue;
    }
    if (buf[0] == 'i') {
      auto v = miner->GetDocMetaDataAfterId(board_name, max_id - k + 1);
      for (auto& data : v) {
        printf("%d (%2d, %2d, %2d) %s\n",
               (int)data.post_time,
               (int)data.num_reply_rows[0],
               (int)data.num_reply_rows[1],
               (int)data.num_reply_rows[2],
               data.title.c_str());
      }
      continue;
    }
    if (buf[0] == 't') {
      auto v = miner->GetDocMetaDataAfterTime(board_name, time(NULL) - k);
      for (auto& data : v) {
        printf("%d (%2d, %2d, %2d) %s\n",
               (int)data.post_time,
               (int)data.num_reply_rows[0],
               (int)data.num_reply_rows[1],
               (int)data.num_reply_rows[2],
               data.title.c_str());
      }
      continue;
    }
  }
}


DocRelInfo FakeAnalyst::GetDocRelInfo(DocIdentity const& idid) const {
  DocIdentity id = idid;
  DocRelInfo ret;

  id.id -= 1; ret.pos_rel_docs.push_back(id);
  id.id -= 1; ret.pos_rel_docs.push_back(id);
  id.id -= 1; ret.pos_rel_docs.push_back(id);

  id.id -= 1; ret.neg_rel_docs.push_back(id);
  id.id -= 1; ret.neg_rel_docs.push_back(id);

  id.id -= 1; ret.neutral_rel_docs.push_back(id);

  return ret;
}

}  // namespace impl_

}  // namespace analyst
