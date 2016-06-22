import bs4
import datetime
import re
import requests

from modules.l2_miner.common import ADocument
from modules.protocol.types import DocMetaData
from modules.protocol.types import DocRealData
from modules.protocol.types import NUM_REPLY_MODES
from modules.protocol.types import ReplyMessage
from modules.protocol.types import ReplyMode


_HEADERS = {
    'Cookie': str('over18=1; __utma=156441338.1052450315.1398943535.1398943535.1398943535.1; __utmb=156441338.2.10.1398943535; __utmc=156441338; __utmz=156441338.1398943535.1.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none)')
}


_REMOVE_TAG_REGULAR = re.compile(r'<.*?>')


class NotFoundException(Exception):
    pass


class Crewer(object):
    def __init__(self, board):
        self._board = board

    def get_document_by_url(self, url):
        try:
            request = requests.get(url, headers=_HEADERS)
            page = bs4.BeautifulSoup(request.text, 'lxml')
            content = page.body.find(id='main-content')

            title = self._get_page_title(page)
            author = self._get_content_author(content)
            post_time = self._get_content_post_time(content)
            reply_rows = self._get_content_reply_rows(content)
            self._clean_content(content)
            content = self._remove_html_tags(str(content))

            meta_data = DocMetaData(
                    0, 0, title, author, post_time, self._board,
                    [len([reply_row
                          for reply_row in reply_rows if reply_row[0] == a])
                     for a in range(NUM_REPLY_MODES)])
            real_data = DocRealData(content, [ReplyMessage(*r)
                                              for r in reply_rows])
            return ADocument(url, meta_data, real_data)
        except Exception as e:
            raise NotFoundException()

    @staticmethod
    def _get_page_title(page):
        return page.title.string

    @staticmethod
    def _get_content_author(content):
        s = content.find(attrs={'class': 'article-meta-value'}).string
        return s.split()[0]

    @staticmethod
    def _get_content_post_time(content):
        a = content.find_all(attrs={'class': 'article-metaline'})[2]
        s = a.find(attrs={'class', 'article-meta-value'}).string
        d = datetime.datetime.strptime(s, '%a %b %d %H:%M:%S %Y')
        return d.timestamp()

    @staticmethod
    def _get_content_reply_rows(content):
        ret = []
        for a in content.find_all(attrs={'class': 'push'}):
            case = a.find(attrs={'class': 'push-tag'})
            if case.string.startswith('推'):
                case = ReplyMode.GOOD
            elif case.string.startswith('噓'):
                case = ReplyMode.WOO
            else:
                case = ReplyMode.NORMAL
            user = a.find(attrs={'class': 'push-userid'}).string
            c = a.find(attrs={'class': 'push-content'}).string[2 : ]
            ret.append((case, user, c))
        return ret

    @staticmethod
    def _clean_content(content):
        classes = ['article-metaline', 'article-metaline-right', 'push', 'f2']
        for c in classes:
            for a in content.find_all(attrs={'class': c}):
                a.decompose()

    @staticmethod
    def _remove_html_tags(data):
        return _REMOVE_TAG_REGULAR.sub('', data)
