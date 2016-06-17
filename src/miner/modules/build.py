# -*- coding: utf-8 -*-
import json
import os
from pprint import pprint
from modules.backend_interface import BackendInterface
from modules.protocol.types import DocMetaData,DocRealData,ReplyMode,ReplyMessage

class BuildData(object):
	def __init__(self,str):
		
		#global metadata
		self.metadata = []
		self.metadatapath = str+"/Metadata_json"
		
		self.d = {}
		# To add a key->value pair, do this:
		
		with open(self.metadatapath) as data_file:
			self.jsondata = json.load(data_file)
		os.chdir(str)
		#pprint(self.jsondata)
		
		for i in range(0,len(self.jsondata)):
			

			Rstr = self.jsondata[i]['Title']
						
			if Rstr in self.d :
				Rid = self.d[Rstr][0]
				# has some values for key
			else:
				Rid = self.jsondata[i]['Id']
				# no values for key
			self.d.setdefault(Rstr, []).append(self.jsondata[i]['Id'])
			"""
			if Rstr in self.mapdata:
				Rid = self.mapdata[Rstr]
				self.mapdata[Rstr].append(self.jsondata[i]['Id'])
			else:
				self.mapdata[Rstr] = self.jsondata[i]['Id']
				Rid = self.jsondata[i]['Id']
			"""	
			#print(self.jsondata[i]['Title'])
			#print(Rid)
			self.metadata.append(
							DocMetaData(self.jsondata[i]['Id'],Rid,
									  self.jsondata[i]['Title'],self.jsondata[i]['Author'],
									  self.jsondata[i]['Time'],self.jsondata[i]['Board'],
									  [self.jsondata[i]['Push'][0],self.jsondata[i]['Push'][1],self.jsondata[i]['Push'][2]])
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
		ansList = []
		for i in range(0,len(self.jsondata)):
			if self.jsondata[i]['Board'] == board and self.jsondata[i]['Id'] == idid :
				#for Rid in self.mapdata[Rstr]:
				if self.jsondata[i]['Title'] in self.d :
					for X in self.d[self.jsondata[i]['Title']]:
						#print(X)
						ansList.append(self.metadata[X])
		return ansList

	def get_doc_real_data(self, board, idid):
		for i in range(0,len(self.jsondata)):
			if self.jsondata[i]['Board'] == board and self.jsondata[i]['Id'] == idid :
				#'http://www.ptt.cc/bbs/' + board + self.jsondata[i]['Name'] + '.html'
				with open(self.jsondata[i]['Name'],'r') as fileFp:
					strr = fileFp.read()
					s = '發信站: 批踢踢實業坊(ptt.cc)'
					List = strr[strr.find(s)+len(s):].split('\n')  # --> ['Line 1', 'Line 2', 'Line 3']
					Reply = []
					LikeChinese = '推'
					DislikeChinese = '噓'
					ArrowChinese = '→'
					#	ReplyMessage(self, mode, user, message):
					for replyString in List:
						if len(replyString) != 0:
							if replyString[0] == LikeChinese :
								Reply.append(ReplyMessage(ReplyMode.GOOD,replyString[2:replyString.find(':')],replyString[replyString.find(':')+1:]))
							elif replyString[0] == ArrowChinese:
								Reply.append(ReplyMessage(ReplyMode.NORMAL,replyString[2:replyString.find(':')],replyString[replyString.find(':')+1:]))
							elif replyString[0] == DislikeChinese:
								Reply.append(ReplyMessage(ReplyMode.WOO,replyString[2:replyString.find(':')],replyString[replyString.find(':')+1:]))
					return DocRealData(strr[strr[1:].find('\n')+2:strr.find(s)-5], Reply)

	def get_id_by_url(self, url):
		print(url[url.rfind('/')+1:url.find('.html')])
		for i in range(0,len(self.jsondata)):
			if self.jsondata[i]['Name'] ==  url[url.rfind('/')+1:url.find('.html')]	:
				return self.jsondata[i]['Id']		
		
	def get_url_by_id(self, board, idid):
		for i in range(0,len(self.jsondata)):
			if self.jsondata[i]['Board'] == board and self.jsondata[i]['Id'] == idid :
				return 'http://www.ptt.cc/bbs/' + board + self.jsondata[i]['Name'] + '.html'
  
