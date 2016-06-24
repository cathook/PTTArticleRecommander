import bs4
import datetime
import logging
import re
import requests
import time

from modules.l2_miner.common import ADocument
from modules.l2_miner.common import HEADERS
from modules.l2_miner.storage import CrewerInterface
from modules.protocol.types import DocMetaData
from modules.protocol.types import DocRealData
from modules.protocol.types import NUM_REPLY_MODES
from modules.protocol.types import ReplyMessage
from modules.protocol.types import ReplyMode


_REMOVE_TAG_REGULAR = re.compile(r'<.*?>')

_TIME_INF = 2 ** 60


class Crewer(CrewerInterface):
    def __init__(self, logger):
        self._logger = logger
        logging.getLogger('urllib3').setLevel(logging.WARNING)

    def get_doc_by_url(self, url):
        meta_data = DocMetaData(
                None, None, '', '', _TIME_INF, None, [0, 0, 0])
        real_data = DocRealData('', [])
        ok = False
        for i in range(30):
            try:
                request = requests.get(url, headers=HEADERS)
                if request.status_code != 200:
                    time.sleep(0.1)
                    continue
                page = bs4.BeautifulSoup(request.text, 'lxml')
                content = page.body.find(id='main-content')

                title = self._get_page_title(page)
                author = self._get_content_author(content)
                post_time = self._get_content_post_time(content)
                reply_rows = self._get_content_reply_rows(content)
                self._clean_content(content)
                content = self._remove_html_tags(str(content))

                meta_data = DocMetaData(
                        None, None, title, author, post_time, None,
                        [len([reply_row
                              for reply_row in reply_rows if reply_row[0] == a])
                         for a in range(NUM_REPLY_MODES)])
                real_data = DocRealData(content,
                                        [ReplyMessage(*r) for r in reply_rows])
                ok = True
            except Exception as _:
                pass
            break
        if not ok:
            self._logger.warning('Cannot fetch document at %s' % url)
        ret = ADocument()
        ret.url = url
        ret.meta_data = meta_data
        ret.real_data = real_data
        return ret

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
            cc = a.find(attrs={'class': 'push-content'}).string
            if cc is not None:
                ret.append((case, user, cc[2 : ]))
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
