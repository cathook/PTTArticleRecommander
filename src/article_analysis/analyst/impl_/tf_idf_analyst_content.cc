#include "tf_idf_analyst_content.h"

#include <math.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cppjieba/Jieba.hpp>
#include <cppjieba/KeywordExtractor.hpp>
#include <opencc/opencc.h>

#include "protocol/types.h"
#include "utils/funcs.h"


using protocol::types::Board;
using protocol::types::Content;
using protocol::types::DocMetaData;
using protocol::types::DocRealData;
using protocol::types::Identity;
using protocol::types::ReplyMessage;
using protocol::types::ReplyMessages;
using protocol::types::ReplyMode;
using std::fill;
using std::map;
using std::string;
using std::swap;
using std::vector;


double const k = 1.5;
double const b = 0.75;

const char* const DICT_PATH = "dict/jieba.dict.utf8";
const char* const HMM_PATH = "dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "dict/user.dict.utf8";
const char* const IDF_PATH = "dict/idf.utf8";
const char* const STOP_WORD_PATH = "dict/stop_words.utf8";


namespace analyst {

namespace impl_ {


char const* const TfIdfAnalystContent::SKIP_CHARS_ =
    ".?!()$,><^-_=|+     *\"\\/[]{};%#`&~@";


TfIdfAnalystContent::TfIdfAnalystContent(miner::Miner *miner,
                                         logging::Logger* parent_logger) {
  logger_ = parent_logger->CreateSubLogger("TF_IDF_Content");

  RunMainAlgo_(miner);
}


TfIdfAnalystContent::~TfIdfAnalystContent() {
  delete logger_;
}


DocRelInfo TfIdfAnalystContent::GetDocRelInfo(DocIdentity const& id) const {
  vector<DocIdentity> relavant;
  int idx = static_cast<int>(id.id) - static_cast<int>(id_base_);
  if (id.board == "Gossiping" &&
      0 <= idx && idx < static_cast<int>(file_calculated_.size()) &&
      file_calculated_[idx]) {
    for (size_t i = 0; i < recommended_file_[idx].size(); ++i) {
      relavant.emplace_back(
          "Gossiping", recommended_file_[idx][i] + id_base_);
    }
    logger_->Info("Handled document %d (%lu).", id.id, relavant.size());
  } else {
    logger_->Info("Cannot answer the query about id %d", id.id);
  }
  return DocRelInfo(relavant, relavant, relavant);
}


void TfIdfAnalystContent::RunMainAlgo_(miner::Miner* miner) {
  int noun_num = 0;
  int final_noun_num = 0;
  int file_count = 0;
  int file_calculated_num = 0;
  vector<int> noun_count;
  vector<int> final_noun_count;
  vector<int> doc_len;
  map<string, int> word_map;
  map<string, int> final_word_map;

  cppjieba::Jieba jieba(utils::GetShareFileFullPath(DICT_PATH),
                        utils::GetShareFileFullPath(HMM_PATH),
                        utils::GetShareFileFullPath(USER_DICT_PATH));
  cppjieba::KeywordExtractor extractor(
      jieba,
      utils::GetShareFileFullPath(IDF_PATH),
      utils::GetShareFileFullPath(STOP_WORD_PATH));

  opencc::SimpleConverter my_opencc("tw2sp.json");

  Board board_name = "Gossiping";

  vector<Content> article_content_vec;

  vector<float> final_IDF;
  vector<float> IDF;
  vector<bool> IDF_add;

  auto all_meta_data = miner->GetDocMetaDataAfterTime(
      board_name, time(NULL) - 24 * 60 * 60);

  logger_->Info("Got all documents from the miner, id in [%d, %d]",
                all_meta_data[0].id, all_meta_data.back().id);

  id_base_ = all_meta_data[0].id;

  for (size_t id = 0; id < all_meta_data.size(); ++id) {
    auto& meta_data = all_meta_data[id];

    int score = (meta_data.num_reply_rows[(int32_t)ReplyMode::GOOD]
                 - meta_data.num_reply_rows[(int32_t)ReplyMode::WOO]);
    if (score > 0) {
      logger_->Info("Add document %lu into account.", id);

      auto doc_real_data = miner->GetDocRealData(board_name, meta_data.id);

      fill(IDF_add.begin(), IDF_add.end(), false);

      SanitizeContent_(&doc_real_data.content);
      doc_real_data.content = my_opencc.Convert(doc_real_data.content);

      // cut all
      vector<string> tagres;
      jieba.CutAll(doc_real_data.content, tagres);

      for(auto it = tagres.begin(); it != tagres.end(); ++it) {
        auto word_it = word_map.find(*it);
        if (word_it == word_map.end()) {
          word_map.emplace(*it, noun_num);
          noun_count.push_back(1);
          ++noun_num;
          IDF.push_back(0.0);
          IDF_add.push_back(true);
        } else {
          ++noun_count[word_it->second];
          if(!IDF_add[word_it->second])
            IDF_add[word_it->second] = true;
        }
      }

      doc_len.push_back(doc_real_data.content.length());
      article_content_vec.push_back(doc_real_data.content);
      file_calculated_.push_back(true);
      ++file_calculated_num;

      for (size_t i = 0; i < IDF_add.size(); ++i)
        if (IDF_add[i])
          IDF[i] += 1.0;
    } else {
      doc_len.push_back(0);
      article_content_vec.push_back("");
      file_calculated_.push_back(false);
    }
    ++file_count;
  }

  logger_->Info("Removes the stopwords.");
  int stop_word = 20;
  for(auto it = word_map.begin(); it != word_map.end(); ++it) {
    if (IDF[it->second] > 2 && IDF[it->second] < stop_word){
      final_word_map.emplace(it->first, final_noun_num);
      final_noun_count.push_back(noun_count[it->second]);
      final_IDF.push_back(IDF[it->second]);
      ++final_noun_num;
    }
  }

  logger_->Info("file num: %d", file_count);
  logger_->Info("calculated file num: %d", file_calculated_num);
  logger_->Info("noun num: %d", noun_num);
  logger_->Info("final noun num: %d", final_noun_num);

  vector<vector<int>> file_word(file_count, vector<int>(final_noun_num));

  int temp_count[final_noun_num];
  int max_noun_count[final_noun_num];

  for (int i = 0; i < final_noun_num; ++i) {
    max_noun_count[i] = 0;
  }

  logger_->Info("Now let's read again.");
  for(size_t sz = 0, size = article_content_vec.size(); sz < size; ++sz) {
    if (!file_calculated_[sz])
      continue;

    for (int i = 0; i < final_noun_num; ++i) {
      temp_count[i] = 0;
    }

    // cut all
    vector<string> tagres;
    jieba.CutAll(article_content_vec[sz], tagres);
    for (auto it = tagres.begin(); it != tagres.end(); ++it) {
      auto word_it = final_word_map.find(*it);
      if(word_it != final_word_map.end()) {
        ++file_word[sz][word_it->second];
        ++temp_count[word_it->second];
      }
    }

    for (int i = 0; i < final_noun_num; i ++) {
      if (temp_count[i] > max_noun_count[i])
        max_noun_count[i] = temp_count[i];
    }
  }

  for (int i = 0; i < final_noun_num; ++i) {
    final_IDF[i] = log(
        (file_calculated_num - final_IDF[i] + 0.5) / (final_IDF[i] + 0.5));
    if (isnan(final_IDF[i])) {
      logger_->Warn("IDF nan");
    }
  }

  double avg_doc_len = 0;
  for (int i = 0; i < file_count; ++i) {
    avg_doc_len += doc_len[i];
  }
  avg_doc_len /= file_calculated_num;

  logger_->Info("Calculate score");

  vector<double> score(file_count);
  for (int l = 0; l < file_count; ++l) {
    if (!file_calculated_[l]) {
      recommended_file_.emplace_back();
      continue;
    }

    fill(score.begin(), score.end(), 0);

    for (int i = 0; i < final_noun_num; ++i) {
      if (file_word[l][i] > 0) {
        for (int j = 0; j < file_count; ++j) {
          if (j == l)
            continue;
          double fw = file_word[j][i];
          double x = fw * (k + 1);
          double y = fw + k * (1 - b + b * doc_len[j] / avg_doc_len);
          score[j] += final_IDF[i] * x / y;
          if (isnan(score[j])) {
            logger_->Warn(
                "Score is nan, hint: avg_len = %f b = %f.", avg_doc_len, b);
          }
        }
      }
    }

    vector<int> best_idx;
    for (int i = 0; i < file_count; ++i) {
      if (i == l) {
        continue;
      }
      best_idx.push_back(i);
      for (int j = static_cast<int>(best_idx.size()) - 1; j - 1 >= 0; --j) {
        if (score[best_idx[j - 1]] >= score[best_idx[j]]) {
          break;
        }
        swap(best_idx[j - 1], best_idx[j]);
      }
      if (best_idx.size() > 3) {
        best_idx.pop_back();
      }
    }
    recommended_file_.emplace_back(best_idx);

    if (l % 1000 == 0)
      logger_->Info("l = %d", l);
  }
}

void TfIdfAnalystContent::SanitizeContent_(string* s) {
  size_t new_size = 0;
  for (size_t i = 0, len = s->length(); i < len; ++i) {
    char c = s->at(i);
    if (strchr(SKIP_CHARS_, c) != NULL) { 
      continue;
    }
    if (c != ':') {
      s->at(new_size++) = c;
    } else {
      while (i < len && s->at(i) != '\n') {
        ++i;
      }
    }
  }
  s->resize(new_size);
}

}  // namespace impl_

}  // namespace analyst
