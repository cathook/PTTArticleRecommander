# -*- coding: utf-8 -*-
import json
from pprint import pprint
from modules.backend_interface import BackendInterface
from modules.protocol.types import DocMetaData
class BuildData(object):
	def __init__(self,str):
		
		#global metadata
		self.metadata = []
		self.metadatapath = str+"/Metadata_json"
		with open(self.metadatapath) as data_file:
			self.jsondata = json.load(data_file)
		

		#pprint(self.jsondata)
		for i in range(0,len(self.jsondata)):
			self.metadata.append(
							DocMetaData(self.jsondata[i]['Id'],self.jsondata[i]['Id'],
									  self.jsondata[i]['Title'],self.jsondata[i]['Author'],
									  self.jsondata[i]['Time'],self.jsondata[i]['Board'],[0,0,0])
							)
			#print(jsondata[i]['Title'])
		#pass
	def get_max_id(self, board):
		cnt = 0
		for i in range(0,len(self.jsondata)):
			if self.jsondata[i]['Board'] == board :
				cnt = cnt + 1
		return cnt

	def get_doc_meta_data_after_id(self, board, idid):
		ansList = []
		for i in range(0,len(self.jsondata)):
			if self.jsondata[i]['Board'] == board and self.jsondata[i]['Id'] == idid :
				ansList.append(self.metadata[i])		
		return ansList

	def get_doc_meta_data_after_time(self, board, post_time):
		ansList = []
		#return ansList
		#	2005-06-21 19:34:53
		for i in range(0,len(self.jsondata)):
			date_object = datetime.strptime(self.jsondata[i]['Time'],'%Y-%m-%d %H:%M:%S')
			if self.jsondata[i]['Board'] == board and post_time > date_object :
				ansList.append(self.metadata[i])
		return ansList

	def get_doc_meta_data_of_author(self, board, author):
		ansList = []
		#return ansList
		#	2005-06-21 19:34:53
		for i in range(0,len(self.jsondata)):
			if self.jsondata[i]['Board'] == board and self.jsondata[i]['Author'] == author :
				ansList.append(self.metadata[i])
		return ansList

	def get_doc_meta_data_series(self, board, idid):
		return []

	def get_doc_real_data(self, board, idid):
		for i in range(0,len(self.jsondata)):
			if self.jsondata[i]['Board'] == board and self.jsondata[i]['Id'] == idid :
				#'http://www.ptt.cc/bbs/' + board + self.jsondata[i]['Name'] + '.html'
				with open(self.jsondata[i]['Name'],'r') as fileFp:
					return DocRealData(fileFp.read(), [])

	def get_id_by_url(self, url):
		print(url[url.rfind('/')+1:url.find('.html')])
		for i in range(0,len(self.jsondata)):
			if self.jsondata[i]['Name'] ==  url[url.rfind('/')+1:url.find('.html')]	:
				return self.jsondata[i]['Id']		
		
	def get_url_by_id(self, board, idid):
		for i in range(0,len(self.jsondata)):
			if self.jsondata[i]['Board'] == board and self.jsondata[i]['Id'] == idid :
				return 'http://www.ptt.cc/bbs/' + board + self.jsondata[i]['Name'] + '.html'
  
