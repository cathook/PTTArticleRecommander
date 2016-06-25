#include "tf_idf_analyst_title.h"

#include <math.h>

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <opencc/opencc.h>
#include <cppjieba/Jieba.hpp>
#include <cppjieba/KeywordExtractor.hpp>

#include "protocol/types.h"
#include "utils/funcs.h"

using std::fill;
using std::map;
using std::string;
using std::swap;
using std::vector;

using protocol::types::Board;
using protocol::types::Content;
using protocol::types::DocMetaData;
using protocol::types::DocRealData;
using protocol::types::Identity;
using protocol::types::ReplyMessage;
using protocol::types::ReplyMessages;
using protocol::types::ReplyMode;


namespace analyst {

namespace impl_ {

namespace {


double const k = 1.5;
double const b = 0.75;

const char* const DICT_PATH = "dict/jieba.dict.utf8";
const char* const HMM_PATH = "dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "dict/user.dict.utf8";
const char* const IDF_PATH = "dict/idf.utf8";
const char* const STOP_WORD_PATH = "dict/stop_words.utf8";

}  // namespace

char const* const TfIdfAnalystTitle::SKIP_CHARS_ =
    ".?!()$,><^-_=|+     *\"\\/[]{};%#`&~@";


TfIdfAnalystTitle::TfIdfAnalystTitle(miner::Miner* miner,
                                     logging::Logger* parent_logger) {
  logger_ = parent_logger->CreateSubLogger("TfIdf");

  RunMainAlgo_(miner);
}

TfIdfAnalystTitle::~TfIdfAnalystTitle() {
  delete logger_;
}

DocRelInfo TfIdfAnalystTitle::GetDocRelInfo(DocIdentity const& id) const {
  vector<DocIdentity> relavant;
  int idx = static_cast<int>(id.id) - static_cast<int>(id_base_);
  if (id.board == "Gossiping" &&
      0 <= idx && idx < static_cast<int>(recommended_file_.size()) &&
      file_calculated_[idx]) {
    for (auto& id : recommended_file_[idx]) {
      relavant.emplace_back("Gossiping", id_base_ + id);
    }
  }
  return DocRelInfo(relavant, relavant, relavant);
}

void TfIdfAnalystTitle::RunMainAlgo_(miner::Miner* miner) {
  int noun_num = 0, final_noun_num = 0, file_count = 0, file_calculated_num = 0;
  vector<int> noun_count;
  vector<int> final_noun_count;
  vector<int> doc_len;
  map<string, int> word_map;
  map<string, int> final_word_map;
  map<string, int>::iterator word_it;

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

  vector<double> final_IDF;
  vector<double> IDF;
  vector<bool> IDF_add;

  logger_->Info("Get meta data of all documents in past 10 days.");
  auto meta_data = miner->GetDocMetaDataAfterTime(
      board_name, time(NULL) - 10 * 24 * 60 * 60);
  id_base_ = meta_data[0].id;

  for (size_t id = 0; id < meta_data.size(); ++id) {
    int opinion = (meta_data[id].num_reply_rows[(int)ReplyMode::GOOD]
                   - meta_data[id].num_reply_rows[(int)ReplyMode::WOO]);
    if (opinion > 0) {
      fill(IDF_add.begin(), IDF_add.end(), false);

      SanitizeContent_(&meta_data[id].title);
      meta_data[id].title = my_opencc.Convert(meta_data[id].title);

      //cut for search
      vector<string> tagres;
      jieba.CutForSearch(meta_data[id].title, tagres);
      for (auto it = tagres.begin(); it != tagres.end(); ++it) {
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

      doc_len.push_back(meta_data[id].title.length());
      article_content_vec.push_back(meta_data[id].title);
      file_calculated_.push_back(true);
      file_calculated_num ++;

      for(size_t i = 0; i < IDF_add.size(); ++id)
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
  int threshold = (int)(file_calculated_num * 0.001);
  int stop_word = (int)(file_calculated_num * 0.1);
  for (auto it = word_map.begin(); it != word_map.end(); ++it) {
    if (noun_count[it->second] > threshold && IDF[it->second] < stop_word) {
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
  vector<int> temp_count(final_noun_num);
  vector<int> max_noun_count(final_noun_num);
  long long len_sum = 0;
  double avg_doc_len;
  for (int i = 0; i < final_noun_num; ++i) {
    max_noun_count[i] = 0;
  }
  for (size_t sz = 0, size = article_content_vec.size(); sz < size; sz ++) {
    if (!file_calculated_[sz])
      continue;

    for (int i = 0; i < final_noun_num; ++i) {
      temp_count[i] = 0;
    }

    // cut for search
    vector<string> tagres;
    jieba.CutForSearch(article_content_vec[sz], tagres);
    for (auto it = tagres.begin(); it != tagres.end(); ++it){
      auto word_it = final_word_map.find(*it);
      if (word_it != final_word_map.end()) {
        ++file_word[sz][word_it->second];
        ++temp_count[word_it->second];
      }
    }

    for (int i = 0; i < final_noun_num; ++i) {
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
  for (int i = 0; i < file_count; ++i) {
    len_sum += doc_len[i];
  }
  avg_doc_len = len_sum / (double)file_calculated_num;

  logger_->Info("Calculate score");
  vector<double> score(file_count);
  for (int l = 0; l < file_count; l ++) {
    if (!file_calculated_[l]) {
      recommended_file_.emplace_back();
      continue;
    }

    for (int i = 0; i < final_noun_num; ++i) {
      if (file_word[l][i] > 0) {
        for (int j = 0; j < file_count; ++j) {
          if (j == l)
            continue;

          double x = file_word[j][i] * (k + 1);
          double y =
              file_word[j][i] + k * (1 - b + b * doc_len[j] / avg_doc_len);

          score[j] += final_IDF[i] * x / y;
          if (isnan(score[j])) {
            logger_->Warn(
                "Nan score, hint: avg_doc_len=%f, y=%f\n", avg_doc_len, y);
          }
        }
      }
    }

    vector<int> best_index;
    for (int i = 0; i < file_count; ++i) {
      if (i == l)
        continue;
      best_index.push_back(i);
      for (int j = static_cast<int>(best_index.size()) - 1; j - 1 >= 0; --j) {
        if (score[best_index[j - 1]] > score[best_index[j]]) {
          break;
        }
        swap(best_index[j - 1], best_index[j]);
      }
      if (best_index.size() > 3) {
        best_index.pop_back();
      }
    }
    recommended_file_.push_back(best_index);
  }
}

void TfIdfAnalystTitle::SanitizeContent_(string* s) {
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
