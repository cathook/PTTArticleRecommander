#include <map>

#include "opencc/opencc.h"
#include "cppjieba/Jieba.hpp"
#include "cppjieba/KeywordExtractor.hpp"
#include "tf_idf_analyst_title.h"
#include "protocol/types.h"

using namespace std;
using protocol::types::Board;
using protocol::types::Identity;
using protocol::types::DocRealData;
using protocol::types::DocMetaData;
using protocol::types::Content;
using protocol::types::ReplyMessages;
using protocol::types::ReplyMessage;
using protocol::types::ReplyMode;

#define k 1.5
#define b 0.75

const char* const DICT_PATH = "/home/student/01/b01902013/IR_final/PTTArticleRecommander/deps/cppjieba/dict/jieba.dict.utf8";
const char* const HMM_PATH = "/home/student/01/b01902013/IR_final/PTTArticleRecommander/deps/cppjieba/dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "/home/student/01/b01902013/IR_final/PTTArticleRecommander/deps/cppjieba/dict/user.dict.utf8";
const char* const IDF_PATH = "/home/student/01/b01902013/IR_final/PTTArticleRecommander/deps/cppjieba/dict/idf.utf8";
const char* const STOP_WORD_PATH = "/home/student/01/b01902013/IR_final/PTTArticleRecommander/deps/cppjieba/dict/stop_words.utf8";

namespace analyst {

namespace impl_ {

TfIdfAnalystTitle::TfIdfAnalystTitle(miner::Miner *miner){
	int noun_num = 0, final_noun_num = 0, file_count = 0, file_calculated_num = 0;
	vector<int> noun_count;
	vector<int> final_noun_count;
	vector<int> doc_len;
	map<string, int> word_map;
	map<string, int> final_word_map;
	map<string, int>::iterator word_it;
	cppjieba::Jieba jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH);
	cppjieba::KeywordExtractor extractor(jieba, IDF_PATH, STOP_WORD_PATH);
	opencc::SimpleConverter my_opencc("tw2sp.json");
	Board board_name = "Gossiping";
	vector<Content> article_content_vec;
	int opinion;
	
	vector<float> final_IDF;
	vector<float> IDF;
	vector<bool> IDF_add;
	fprintf(stderr, "read first time\n");
	vector<DocMetaData> meta_data;
	meta_data = miner->GetDocMetaDataAfterTime(board_name, time(NULL) - 1 * 24 * 60 * 60);
	id_base = meta_data[0].id;

	string puntuation = ".?!()$,><^-_=|+	 *\"\\/[]{};%#`&~@";
	for (unsigned id = 0; id < meta_data.size(); id ++){
		
		opinion = meta_data[id].num_reply_rows[(int)ReplyMode::GOOD] - meta_data[id].num_reply_rows[(int)ReplyMode::WOO];
		if(opinion > 0){
			for(unsigned i = 0; i < IDF_add.size(); i ++)
				IDF_add[i] = false;
			string final = "";
			for(unsigned i = 0; i < meta_data[id].title.length(); i ++){
				bool skip = false;
				for(unsigned j = 0; j < puntuation.length(); j ++){
					if(meta_data[id].title[i] == puntuation[j]){
						skip = true;
						break;
					}
				}
				if(!skip){
					if(meta_data[id].title[i] != ':'){
						final += meta_data[id].title[i];
					}
					else{
						while(i < meta_data[id].title.length() && meta_data[id].title[i] != '\n')
							i ++;
					}
				
				}
			}
			meta_data[id].title = my_opencc.Convert(final);
			
			//n words
			//vector<pair<string, string> > tagres;
			//jieba.Tag(meta_data[id].title, tagres);
			//for(vector<pair<string, string> >::iterator it = tagres.begin(); it != tagres.end(); it++){
				//if(it->second == "n"){
			 		//word_it = word_map.find(it->first);
			 		//if(word_it == word_map.end()){
			 			//word_map.insert(pair<string, int>(it->first, noun_num));
			 			//noun_count.push_back(1);
			 			//noun_num ++;
						//IDF.push_back(0.0);
						//IDF_add.push_back(true);
			 		//}
			 		//else{
			 			//noun_count[word_it->second] ++;
						//if(!IDF_add[word_it->second])
							//IDF_add[word_it->second] = true;
			 		//}
			 	//}
			 //}

			//keywords
			//vector<cppjieba::KeywordExtractor::Word> keywordres;
			//extractor.Extract(meta_data[id].title, keywordres, topk);

			//for(vector<cppjieba::KeywordExtractor::Word>::iterator it = keywordres.begin(); it != keywordres.end(); it++){
			 	//word_it = word_map.find(it->word);
			 	//if(word_it == word_map.end()){
			 		//word_map.insert(pair<string, int>(it->word, noun_num));
			 		//noun_count.push_back(1);
			 		//noun_num ++;
					//IDF.push_back(0.0);
					//IDF_add.push_back(true);
			 	//}
			 	//else{
			 		//noun_count[word_it->second] ++;
					//if(!IDF_add[word_it->second])
						//IDF_add[word_it->second] = true;
			 	//}
			//}

			//cut for search
			vector<string> tagres;
			jieba.CutForSearch(meta_data[id].title, tagres);
			for(vector<string>::iterator it = tagres.begin(); it != tagres.end(); it++){
				word_it = word_map.find(*it);
				if(word_it == word_map.end()){
					word_map.insert(pair<string, int>(*it, noun_num));
					noun_count.push_back(1);
					noun_num ++;
					IDF.push_back(0.0);
					IDF_add.push_back(true);
				}
				else{
					noun_count[word_it->second] ++;
					if(!IDF_add[word_it->second])
						IDF_add[word_it->second] = true;
				}
			}

			//cut all
			// vector<string> tagres;
			// jieba.CutAll(meta_data[id].title, tagres);
			// for(vector<string>::iterator it = tagres.begin(); it != tagres.end(); it++){
			//  	word_it = word_map.find(*it);
			//  	if(word_it == word_map.end()){
			//  		word_map.insert(pair<string, int>(*it, noun_num));
			//  		noun_count.push_back(1);
			//  		noun_num ++;
			// 		IDF.push_back(0.0);
			// 		IDF_add.push_back(true);
			//  	}
			//  	else{
			//  		noun_count[word_it->second] ++;
			// 		if(!IDF_add[word_it->second])
			// 			IDF_add[word_it->second] = true;
			//  	}
			// }


			doc_len.push_back(meta_data[id].title.length());
			article_content_vec.push_back(meta_data[id].title);
			file_calculated.push_back(true);
			file_calculated_num ++;

			for(unsigned i = 0; i < IDF_add.size(); i ++)
				if(IDF_add[i])
					IDF[i] += 1.0;
		}
		else{
			doc_len.push_back(0);
			article_content_vec.push_back("");
			file_calculated.push_back(false);
		}
		file_count ++;
	}

	fprintf(stderr, "read second time\n");
	int threshold = (int)(file_calculated_num * 0.001);
	int stop_word = (int)(file_calculated_num * 0.1);
	for(map<string, int>::iterator it = word_map.begin(); it != word_map.end(); it++){
		if(noun_count[it->second] > threshold && IDF[it->second] < stop_word){
			final_word_map.insert(pair<string, int>(it->first, final_noun_num));
			final_noun_count.push_back(noun_count[it->second]);
			final_IDF.push_back(IDF[it->second]);
			final_noun_num ++;
		}
	}
	printf("file num: %d\n", file_count);
	printf("calculated file num: %d\n", file_calculated_num);
	printf("noun num: %d\n", noun_num);
	printf("final noun num: %d\n\n", final_noun_num);

	vector<vector<int> > file_word(file_count, vector<int>(final_noun_num));
	//int file_word[file_count][final_noun_num];
	int temp_count[final_noun_num];
	int max_noun_count[final_noun_num];
	long long len_sum = 0;
	float avg_doc_len;
	for(int i = 0; i < final_noun_num; i ++){
		max_noun_count[i] = 0;
	}
	vector<int>::size_type size = article_content_vec.size();
	for(unsigned sz = 0; sz < size; sz ++){
		if(!file_calculated[sz])
			continue;

		for(int i = 0; i < final_noun_num; i ++){
			temp_count[i] = 0;
		}

		// n
		//vector<pair<string, string> > tagres;
		//jieba.Tag(article_content_vec[sz], tagres);
		//for(vector<pair<string, string> >::iterator it = tagres.begin(); it != tagres.end(); it++){
			//if(it->second == "n"){
		 		//word_it = final_word_map.find(it->first);
		 		//if(word_it != final_word_map.end()){
					//printf("%u\n", sz);
		 			//file_word[sz][word_it->second] ++;
		 			//temp_count[word_it->second] ++;
		 		//}
		 	//}
		//}

		// keywords
		//vector<cppjieba::KeywordExtractor::Word> keywordres;
		//extractor.Extract(article_content_vec[sz], keywordres, topk);
		//for(vector<cppjieba::KeywordExtractor::Word>::iterator it = keywordres.begin(); it != keywordres.end(); it++){
			//word_it = final_word_map.find(it->word);
		 	//if(word_it != final_word_map.end()){
		 		//file_word[sz][word_it->second] ++;
		 		//temp_count[word_it->second] ++;
		 	//}
		//}

		// cut for search
		vector<string> tagres;
		jieba.CutForSearch(article_content_vec[sz], tagres);
		for(vector<string>::iterator it = tagres.begin(); it != tagres.end(); it++){
			word_it = final_word_map.find(*it);
			if(word_it != final_word_map.end()){
				file_word[sz][word_it->second] ++;
				temp_count[word_it->second] ++;
			}
		}

		// cut all
		// vector<string> tagres;
		// jieba.CutAll(article_content_vec[sz], tagres);
		// for(vector<string>::iterator it = tagres.begin(); it != tagres.end(); it++){
		// 	word_it = final_word_map.find(*it);
		//  	if(word_it != final_word_map.end()){
		//  		file_word[sz][word_it->second] ++;
		//  		temp_count[word_it->second] ++;
		//  	}
		// }

		for(int i = 0; i < final_noun_num; i ++){
			if(temp_count[i] > max_noun_count[i])
				max_noun_count[i] = temp_count[i];
		}
	}

	for(int i = 0; i < final_noun_num; i ++){
		final_IDF[i] = log((file_calculated_num - final_IDF[i] + 0.5)/(final_IDF[i] + 0.5));
		if(final_IDF[i] != final_IDF[i]){
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
				for(int j = 0; j < file_count; j ++){
					if(j == l)
						continue;
					score[j] += final_IDF[i] * ((float)file_word[j][i]*(k+1) / ((float)file_word[j][i]+k*(1-b+b*doc_len[j]/avg_doc_len)));
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
		//printf("%d\n", l+1);
		vector<int> temp_push;
		for(int i = 0; i < 3; i ++){
			if(max_index[i] != -1){
				//printf("%d ", max_index[i]+1);
				temp_push.push_back(max_index[i]);
			}
		}
		//printf("\n\n");
		recommended_file.push_back(temp_push);
		if(l % 1000 == 0)
			fprintf(stderr, "%d\n", l+1);
	}
}
DocRelInfo TfIdfAnalystTitle::GetDocRelInfo(DocIdentity const& id) const{
	vector<DocIdentity> relavant;
	DocIdentity temp;
	if(file_calculated[id.id - id_base]){
		for(unsigned i = 0; i < recommended_file[id.id - id_base].size(); i ++){
			temp.id = recommended_file[id.id - id_base][i];
			temp.board = "Gossiping";
			relavant.push_back(temp);
		}
	}	
	return DocRelInfo(relavant, relavant, relavant);
}

}  // namespace impl_

}  // namespace analyst

