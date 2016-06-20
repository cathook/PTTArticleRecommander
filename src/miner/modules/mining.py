import bs4
#import urllib2
import re
import time
import logging
import json
from datetime import datetime
from optparse import OptionParser
from optparse import Option, OptionValueError
import urllib.request as urllib2
import os, sys
import importlib
importlib.reload(sys)

LOGNAME = 'log'
VERSION = '0.2'
globvar = 0
#arr = []
class BBSCrawler(object):
    '''
    @author: Paul Yang
    @note: This prog is to fetch the ptt's content based on the board name like car  and the fetched files will be stored under the directory "./fetched/BOARDNAME/"
    @since: 2014/8/2, v0.2
    '''

    def __init__(self, board_name = 'car', myPageNum = 10, fetch_path = './', toNum = 0,debugFlag = False, forAll = False):
        '''
        Constructor
        '''
        self.useHeader = False
        ## debug flag to enable debug - not finished yet.
        self.debugFlag = debugFlag
        self.board_name = board_name
        
        ## put the cookie header for the board like Gossiping to pass around the limit of 18 age 
        if self.board_name == 'Gossiping':
            self.initHeader()
            self.useHeader = True
        
        self.myPageNum = myPageNum
        
        ## if forAll is on, iterate the total number of pages for the board by getAllPagesInTheBoard()    
        self.forAll = forAll
        self.toNum = toNum
        self.fetch_path = fetch_path
        self.path = os.path.join(self.fetch_path, self.board_name)
        self.ESPECIAL_URL = 'http://www.ptt.cc/bbs/' + self.board_name + '/index' + '.html'
        self.post_url = lambda id: 'http://www.ptt.cc/bbs/' + self.board_name + '/' + id + '.html'
        self.page_url = lambda n: 'http://www.ptt.cc/bbs/' + self.board_name + '/index' + str(n) + '.html'
        self.initLogging()
        self.statisticDic = dict()
        self.num_pushes = dict()
        self.metadic = dict()
        if not os.path.exists(self.path):
            os.makedirs(self.path)
        os.chdir(self.path)
        sys.stderr.write('Crawling "%s" ...\n' % self.board_name)
        self.logger.info('Crawling "%s" ...\n' % self.board_name)
        
        
    ## for over 18 content, need to put the header 
    def initHeader(self): 
        self.headers = dict()
        self.headers['Cookie'] = str('over18=1; __utma=156441338.1052450315.1398943535.1398943535.1398943535.1; __utmb=156441338.2.10.1398943535; __utmc=156441338; __utmz=156441338.1398943535.1.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none)' )
        
        
    def initLogging(self):
        '''
        initializing logging function and put to /bbsCrawler.log
        '''
        print ("initializing the logging .......")
        myLogPath = os.path.join(self.path, LOGNAME)
        try:
            os.makedirs(myLogPath)
        except: 
            sys.stderr.write('Warning: "%s" already existed\n' % myLogPath)
        LOGPATH = myLogPath + '/bbsCrawler.log'
        #logger.warn('Warning: "%s" already existed\n' % myLogPath)
        self.logger = logging.getLogger('bbs crawler')
        self.logger.setLevel(logging.DEBUG)
        formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
        hdlr = logging.FileHandler(LOGPATH)
        hdlr.setFormatter(formatter)
        self.logger.addHandler(hdlr) 
        #logger.setLevel(logging.DEBUG)
        self.logger.info('bbs crawler started')
    
    def remove_html_tags(self, data):
        p = re.compile(r'<.*?>')
        return p.sub('', data)
    
    def closeLogging(self):
        self.logger.info('closing logging')
        handlers = self.logger.handlers[:]
        for handler in handlers:
            handler.close()
            self.logger.removeHandler(handler)
    
    def getAllPagesInTheBoard(self):
        self.logger.info('getting all pages number from "%s" ...\n' % self.board_name)
        try:
            if (self.useHeader):
                request = urllib2.Request(self.ESPECIAL_URL, headers=self.headers)
                indexPage = bs4.BeautifulSoup(urllib2.urlopen(request).read(), "lxml")
            else:
                indexPage = bs4.BeautifulSoup(urllib2.urlopen(self.ESPECIAL_URL).read(), "lxml")
            ## filter '/bbs/car/index1275.html' to number only "1275"
            self.allPageNums = int(re.sub(r'[^0-9]+', '', indexPage.find_all("a", class_="btn wide")[1].get('href')))
            sys.stderr.write('Total number of pages: %d\n' % self.allPageNums)
            self.logger.error('Total number of pages: %d\n' % self.allPageNums)
            global globvar    
            globvar = self.allPageNums
        except:
            sys.stderr.write('can not get the number of pages')
            self.logger.error('cannot get the number of pages \n')
    
    def getContent(self):
        start_time = time.time()
        
        if (not self.forAll): 
            ##use self.myPageNum for designate page 
            pagesToRun = self.myPageNum
        else:
            ##use all number got from getAllPagesInTheBoard()
            pagesToRun = self.allPageNums
        
        if (self.toNum != 0 and not self.forAll): ## to suport 2 arguments start 3 -> 100
            startIndex = pagesToRun
            endIndex = self.toNum
        else:
            startIndex = 1
            endIndex = pagesToRun
        

        ## add the index to record the total number that has processed, the number of failure and the number of success
        self.statisticDic['indexFailure'] = 0
        self.statisticDic['totalPostNum'] = 0
        self.statisticDic['fetchFailureNum'] = 0

        ID = 1
        self.arr = []
        with open('Metadata_json','w+') as metaDataFp:
            ## iterate through index page like "www.ptt.cc/bbs/car/index.html" to get each POST ID  
            #for indexP in xrange(1, pagesToRun + 1):
            for indexP in range(startIndex, endIndex):
                sys.stderr.write('start from index %s ...\n' % indexP)
                self.logger.debug('start from index %s ...\n' % indexP)
                try:
                    if (self.useHeader): ## if the page require header 
                        request = urllib2.Request(self.page_url(indexP), headers=self.headers)
                        page = bs4.BeautifulSoup(urllib2.urlopen(request).read(), "lxml")
                    else:
                        page = bs4.BeautifulSoup(urllib2.urlopen(self.page_url(indexP)).read(), "lxml")
                except:
                    sys.stderr.write('Error occured while fetching %s\n' % self.page_url(indexP))
                    self.logger.error('Error occured while fetching %s\n' % self.page_url(indexP))
                    ## how many index has failed 
                    self.statisticDic['indexFailure'] += 1
                    continue
                
                ## iterate through posts on this page
                for link in page.find_all(class_='r-ent'):
                    
                    try: 
                        ## For instance: "M.1368632629.A.AF7"
                        post_id = link.a.get('href').split('/')[-1][:-5]
                        """
                        ## Record the number of pushes from <div class="nrec">, which is an integer from -100 to 100
                        if (link.span):
                            if link.span.contents[0] == u'爆' :
                                self.num_pushes[post_id] = 100
                            elif link.span.contents[0] == "X1" :
                                self.num_pushes[post_id] = -10
                            elif link.span.contents[0] == "X2" :
                                self.num_pushes[post_id] = -20
                            elif link.span.contents[0] == "X3" :
                                self.num_pushes[post_id] = -30
                            elif link.span.contents[0] == "X4" :
                                self.num_pushes[post_id] = -40
                            elif link.span.contents[0] == "X5" :
                                self.num_pushes[post_id] = -50
                            elif link.span.contents[0] == "X6" :
                                self.num_pushes[post_id] = -60
                            elif link.span.contents[0] == "X7" :
                                self.num_pushes[post_id] = -70
                            elif link.span.contents[0] == "X8" :
                                self.num_pushes[post_id] = -80
                            elif link.span.contents[0] == "X9" :
                                self.num_pushes[post_id] = -90
                            elif link.span.contents[0] == "XX" :
                                self.num_pushes[post_id] = -100
                            else:
                                self.num_pushes[post_id] = int(link.span.contents[0])
                        ## if can't find push, set 0 push 
                        else:
                            self.num_pushes[post_id] = 0
                        """
                    except:
                        sys.stderr.write('Error occured while fetching2 %s\n' % post_id)
                        self.logger.error('Error occured while fetching %s\n' % post_id)
                        continue
                    
                    ## Fetch the post content via post id, ex. http://www.ptt.cc/bbs/car/M.1400136465.A.DD5.html     
                    self.statisticDic['totalPostNum'] += 1
                    try:
                                                
                        sys.stderr.write('Fetching %s ...\n' % post_id)
                        self.logger.info('Fetching %s ...\n' % post_id)
                        if (self.useHeader): ## if the page require header 
                            request = urllib2.Request(self.post_url(post_id), headers=self.headers)
                            post = bs4.BeautifulSoup(urllib2.urlopen(request).read(), "lxml")
                        else:
                            post = bs4.BeautifulSoup(urllib2.urlopen(self.post_url(post_id)).read(), "lxml")
                    except:
                        sys.stderr.write('Error occured while fetching %s\n' % self.post_url(post_id))
                        self.logger.error('Error occured while fetching %s\n' % self.post_url(post_id))
                        ##self.fetchFailureNum += 1
                        self.statisticDic['fetchFailureNum'] += 1
                        continue
        
                    with open(post_id, 'w') as contentFile_fp:
                        try:
                            strr = self.remove_html_tags(str(post.find(id='main-container')))
                            s = '發信站: 批踢踢實業坊(ptt.cc)'
                            List = strr[strr.find(s)+len(s):].split('\n')  # --> ['Line 1', 'Line 2', 'Line 3']
                            like = 0
                            dislike = 0
                            arrow = 0
                            LikeChinese = '推'
                            DislikeChinese = '噓'
                            ArrowChinese = '→'  
                            for replyString in List:
                                if len(replyString) != 0:
                                    if replyString[0] == LikeChinese :
                                        like = like + 1
                                    elif replyString[0] == ArrowChinese:
                                        arrow = arrow + 1
                                    elif replyString[0] == DislikeChinese:
                                        dislike = dislike + 1
                            #print([like,arrow,dislike])        
                            #if s in strr:
                                #contentFile_fp.write(strr[0:strr.find(s)-5])
                            #else:
                                #contentFile_fp.write(strr)
                            contentFile_fp.write(strr[strr[1:].find('\n')+2:])
                            #contentFile_fp.write(strr)
                            contentFile_fp.write('\n')
                            #contentFile_fp.write(self.remove_html_tags(str(post.find(id='main-container'))))
                            
                            spans = post.find_all('span', {'class' : 'article-meta-value'})
                            count = 1
                            #self.num_pushes[post_id] = int(link.span.contents[0])
                            metaID= ID
                            metaName= str(post_id)
                            metaPush = [like, arrow, dislike]
                            #contentFile_fp.write(str(post_id)+"\n")
                            #contentFile_fp.write(str(link.span.contents[0])+"\n")
                            for span in spans:
                                #print(span.string)
                                if count == 1:
                                    metaAuthor = span.string[0:span.string.find('(')-1]
                                if count == 2:
                                    metaBoard = span.string
                                if count == 3:                                    
                                    if span.string.find("Re:") == -1:
                                        metaTitle = span.string
                                    else:
                                        metaTitle = span.string[4:]
                                if count == 4:                          
                                    date_object = datetime.strptime(span.string[4:], '%b %d %H:%M:%S %Y')
                                    #contentFile_fp.write(str(date_object) + '\n') ## write title in a first line
                                    metaTime = str(date_object)
                                #else:
                                    #contentFile_fp.write(span.string) ## write title in a first line
                                    #contentFile_fp.write('\n')
                                count = count + 1
                        except:
                            sys.stderr.write('Error occured while fetching4 %s\n' % metaName)
                            continue
                        contentFile_fp.close()


                    os.chdir(self.fetch_path)
                    # delay for a little while in fear of getting blocked
                    time.sleep(0.1)
                    # json.dump({'Id':metaID,'Name':metaName, 'Push':metaPush, 'Author':metaAuthor, 'Board':metaBoard, 'Title':metaTitle,'Time':metaTime}, metaDataFp, indent=7, ensure_ascii=False)
                    self.arr.append({'Id':metaID,'Name':metaName, 'Push':metaPush, 'Author':metaAuthor, 'Board':metaBoard, 'Title':metaTitle,'Time':metaTime})
                    ID = ID + 1
            json.dump(self.arr, metaDataFp, indent=7, ensure_ascii=False)
            metaDataFp.close()
            time.sleep(0.2)

            
        ## dump the number of pushes mapping to the file 'num_pushes_json'
        """
        with open('num_pushes_json', 'w') as numPushesFp, open('metadata_dic_json', 'w') as metadataDicFp:
            self.logger.info('Saving the metadata dic and push mapping into JSON')
            #numPushesFp = open('num_pushes_json', 'w')
            #metadataDicFp = open('metadata_dic_json', 'w')
            json.dump(self.num_pushes, numPushesFp)
            json.dump(self.metadic, metadataDicFp)
            numPushesFp.close()
            metadataDicFp.close()
        """
            
        ## do the final logging and printing all numbers     
        self.logger.info('Ending crawling "%s" ... !! \n' % self.board_name)
        self.logger.info('\n')
        self.logger.info('Statistic: \n')
        self.logger.info('indexFailure number: "%s" \n' % self.statisticDic['indexFailure'])
        self.logger.info('totalPost number: "%s" \n' % self.statisticDic['totalPostNum'])
        self.logger.info('fetchFailure number: "%s"  \n' % self.statisticDic['fetchFailureNum'])
        self.logger.info('\n')
        os.chdir(self.fetch_path)
        print( "the dir is: %s" %os.listdir(os.getcwd()))
        self.closeLogging()
        #os.rename(self.board_name,self.board_name + "_" + str(self.myPageNum) + "_" + str(self.toNum))
        elapsed_time = time.time() - start_time
        print ("the dir is: %s" %os.listdir(os.getcwd()))
        print ("the total post num: %s" % self.statisticDic['totalPostNum'])
        print ("elapsed time: %s" % elapsed_time)
        

class MultipleOption(Option):
    ACTIONS = Option.ACTIONS + ("extend",)
    STORE_ACTIONS = Option.STORE_ACTIONS + ("extend",)
    TYPED_ACTIONS = Option.TYPED_ACTIONS + ("extend",)
    ALWAYS_TYPED_ACTIONS = Option.ALWAYS_TYPED_ACTIONS + ("extend",)

    def take_action(self, action, dest, opt, value, values, parser):
        if action == "extend":
            values.ensure_value(dest, []).append(value)
        else:
            Option.take_action(self, action, dest, opt, value, values, parser)   
