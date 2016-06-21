#include <map>

#include "opencc/opencc.h"
#include "cppjieba/Jieba.hpp"
#include "cppjieba/KeywordExtractor.hpp"
#include "tf_idf_analyst.h"
#include "protocol/types.h"
#include "protocol/types.h"

using namespace std;
using protocol::types::Board;
using protocol::types::Identity;
using protocol::types::DocRealData;
using protocol::types::Content;
using protocol::types::ReplyMessages;
using protocol::types::ReplyMessage;
using protocol::types::ReplyMode;

#define k 1.5
#define b 0.75

const char* const DICT_PATH = "/home/student/01/b01902013/IR_final/PTTArticleRecommander/deps/cppjieba/dict/jieba.dict.utf8";
const char* const HMM_PATH = "/home/student/01/b01902013/IR_final/PTTArticleRecommander/deps/cppjieba/dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "/home/student/01/b01902013/IR_final/PTTArticleRecommander/deps/cppjieba/dict/user.dict.utf8";

namespace analyst {

namespace impl_ {

TfIdfAnalyst::TfIdfAnalyst(miner::Miner *miner){
	int noun_num = 0, final_noun_num = 0, file_count = 0, file_calculated_num = 0;
	vector<int> noun_count;
	vector<int> final_noun_count;
	vector<int> doc_len;
	map<string, int> word_map;
	map<string, int> final_word_map;
	map<string, int>::iterator word_it;
	cppjieba::Jieba jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH);
	opencc::SimpleConverter my_opencc("tw2sp.json");
	Board board_name = "Gossiping";
	Identity max_id;
	DocRealData doc_real_data;
	vector<Content> article_content_vec;
	max_id = miner->GetMaxId(board_name);
	int opinion;

	fprintf(stderr, "read first time\n");
	for (Identity id = 0; id < max_id; id ++){
		doc_real_data = miner->GetDocRealData(board_name, id);
		
		opinion = 0;
		for(vector<ReplyMessage>::iterator message_it = doc_real_data.reply_messages.begin(); 
			message_it != doc_real_data.reply_messages.end(); message_it ++){
			if(message_it->mode == ReplyMode::GOOD)
				opinion ++;
			else if(message_it->mode == ReplyMode::WOO)
				opinion --;
		}

		if(opinion > 0){
			vector<pair<string, string> > tagres;
			doc_real_data.content = my_opencc.Convert(doc_real_data.content);
			jieba.Tag(doc_real_data.content, tagres);
			for(vector<pair<string, string> >::iterator it = tagres.begin(); it != tagres.end(); it++){
				if(it->second == "n"){
					word_it = word_map.find(it->first);
					if(word_it == word_map.end()){
						word_map.insert(pair<string, int>(it->first, noun_num));
						noun_count.push_back(1);
						noun_num ++;
					}
					else{
						noun_count[word_it->second] ++;
					}
				}
			}
			doc_len.push_back(doc_real_data.content.length());
			article_content_vec.push_back(doc_real_data.content);
			file_calculated.push_back(true);
			file_calculated_num ++;
		}
		else{
			doc_len.push_back(0);
			article_content_vec.push_back("");
			file_calculated.push_back(false);
		}
		file_count ++;
	}

	fprintf(stderr, "read second time\n");
	int threshold = (int)(file_calculated_num * 0.01);
	for(map<string, int>::iterator it = word_map.begin(); it != word_map.end(); it++)
		if(noun_count[it->second] > threshold){
			final_word_map.insert(pair<string, int>(it->first, final_noun_num));
			final_noun_count.push_back(noun_count[it->second]);
			final_noun_num ++;
		}
	printf("file num: %d\n", file_count);
	printf("calculated file num: %d\n", file_calculated_num);
	printf("noun num: %d\n", noun_num);
	printf("final noun num: %d\n\n", final_noun_num);

	vector<vector<int> > file_word(file_count, vector<int>(final_noun_num));
	//int file_word[file_count][final_noun_num];
	int temp_count[final_noun_num];
	int max_noun_count[final_noun_num];
	float IDF[final_noun_num];
	long long len_sum = 0;
	float avg_doc_len;
	bool IDF_add[final_noun_num];
	for(int i = 0; i < final_noun_num; i ++){
		max_noun_count[i] = 0;
		IDF[i] = 0;
	}
	vector<int>::size_type size = article_content_vec.size();
	for(unsigned sz = 0; sz < size; sz ++){
		if(!file_calculated[sz])
			continue;

		for(int i = 0; i < final_noun_num; i ++){
			temp_count[i] = 0;
			IDF_add[i] = false;
		}

		vector<pair<string, string> > tagres;
		jieba.Tag(article_content_vec[sz], tagres);

		for(vector<pair<string, string> >::iterator it = tagres.begin(); it != tagres.end(); it++){
			if(it->second == "n"){
				word_it = final_word_map.find(it->first);
				if(word_it != final_word_map.end()){
					file_word[sz][word_it->second] ++;
					temp_count[word_it->second] ++;
					if(!IDF_add[word_it->second])
						IDF_add[word_it->second] = true;
				}
			}
		}
		for(int i = 0; i < final_noun_num; i ++){
			if(temp_count[i] > max_noun_count[i])
				max_noun_count[i] = temp_count[i];
			if(IDF_add[i])
				IDF[i] += 1.0;
		}
	}

	for(int i = 0; i < final_noun_num; i ++){
		IDF[i] = log((file_calculated_num - IDF[i] + 0.5)/(IDF[i] + 0.5));
		if(IDF[i] != IDF[i]){
			printf("IDF nan\n");
		}
	}
	for(int i = 0; i < file_count; i ++){
		len_sum += doc_len[i];
	}
	avg_doc_len = len_sum / (float)file_calculated_num;

	fprintf(stderr, "calculate score\n");
	float score[file_count], temp_max[3];
	int max_index[3];
	for(int l = 0; l < file_count; l ++){
		if(!file_calculated[l])
			continue;

		for(int i = 0; i < 3; i ++){
			temp_max[i] = 0.0;
			max_index[i] = -1;
		}
		for(int i = 0; i < file_count; i ++)
			score[i] = 0.0;
		for(int i = 0; i < final_noun_num; i ++){
			if(file_word[l][i] > 0){
				for(int j = 0; j < file_count && j != l; j ++){
					score[j] += IDF[i] * ((float)file_word[j][i]*(k+1) / ((float)file_word[j][i]+k*(1-b+b*doc_len[j]/avg_doc_len)));
					if(score[j] != score[j]){
						printf("%f\n", avg_doc_len);
						printf("%f\n", ((float)file_word[j][i]+k*(1-b+b*doc_len[j]/avg_doc_len)));
						printf("score nan\n");
					}	
				}
			}
		}
		for(int i = 0; i < file_count; i ++){
			if(i == l)
				continue;
			if(score[i] > temp_max[0]){
				for(int j = 2; j > 0; j --){
					max_index[j] = max_index[j - 1];
					temp_max[j] = temp_max[j - 1];
				}
				max_index[0] = i;
				temp_max[0] = score[i];
			}
			else if(score[i] > temp_max[1]){
				for(int j = 2; j > 1; j --){
					max_index[j] = max_index[j - 1];
					temp_max[j] = temp_max[j - 1];
				}
				max_index[1] = i;
				temp_max[1] = score[i];
			}
			else if(score[i] > temp_max[2]){
				max_index[2] = i;
				temp_max[2] = score[i];
			}
		}
		printf("%d\n", l+1);
		vector<int> temp_push;
		for(int i = 0; i < 3; i ++){
			if(max_index[i] != -1){
				printf("%d ", max_index[i]+1);
				temp_push.push_back(max_index[i]);
			}
		}
		printf("\n\n");
		recommended_file.push_back(temp_push);
		//if(l % 1000 == 0)
		//	fprintf(stderr, "%d\n", l+1);
	}
}
DocRelInfo TfIdfAnalyst::GetDocRelInfo(DocIdentity const& id) const{
	vector<DocIdentity> relavant;
	DocIdentity temp;
	if(file_calculated[id.id - 1]){
		for(unsigned i = 0; i < recommended_file[id.id - 1].size(); i ++){
			temp.id = recommended_file[id.id - 1][i] + 1;
			temp.board = "Gossiping";
			relavant.push_back(temp);
		}
	}	
	return DocRelInfo(relavant, relavant, relavant);
}

}  // namespace impl_

}  // namespace analyst

