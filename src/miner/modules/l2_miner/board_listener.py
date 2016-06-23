import bs4
import random
import requests
import threading
import time

from modules.l2_miner.common import HEADERS
from modules.l2_miner.common import SERVER_ADDR
from modules.l2_miner.common import URL_PREFIX


class BoardCacheInterface(object):
    @property
    def newest_url(self):
        pass

    def add_urls(self, num_urls, urls_gen):
        pass


class _BoardListenerUrlGenerator(object):
    def __init__(self, urls_getter, url_num_getter, min_url_num, max_page_idx):
        self._urls_getter = urls_getter
        self._url_num_getter = url_num_getter

        self._min_url_num = min_url_num
        self._curr_page_idx = max_page_idx + 1
        self._curr_page_urls = []
        self._prev_url = None

    def __iter__(self):
        return self

    def __next__(self):
        while self._curr_page_idx > 1 and not self._curr_page_urls:
            self._curr_page_idx -= 1
            self._curr_page_urls = self._urls_getter(self._curr_page_idx)
        if not self._curr_page_urls:
            return self._prev_url[ : -len('.X.XXX.html')] + '_x'
        if self._url_num_getter(self._curr_page_urls[-1]) <= self._min_url_num:
            return self._prev_url[ : -len('.X.XXX.html')] + '_x'
        curr = self._curr_page_urls[0]
        self._curr_page_urls = self._curr_page_urls[1 : ]
        self._prev_url = SERVER_ADDR + curr
        return SERVER_ADDR + curr


class BoardListener(threading.Thread):
    def __init__(self, board_name, update_timestamp, cache):
        super(BoardListener, self).__init__()
        self.daemon = True

        self._board_name = board_name
        self._url_prefix = URL_PREFIX + board_name + '/'
        self._update_timestamp = update_timestamp
        self._cache = cache
        self._stop_flag = False

    def run(self):
        while not self._stop_flag:
            url_num = self._get_url_compare_num(self._cache.newest_url)
            max_page_idx = self._get_max_page_idx()

            n = self._estimate_num_urls_between(url_num, max_page_idx)
            g = _BoardListenerUrlGenerator(self._get_page_urls,
                                           self._get_url_compare_num,
                                           url_num,
                                           max_page_idx)
            if n > 0:
                self._cache.add_urls(n, g)
            time.sleep(self._update_timestamp)

    def stop(self):
        self._stop_flag = True

    def _get_url_compare_num(self, url):
        if url is None:
            return 0
        a = url[url.find(self._board_name) + len(self._board_name) + 3: ]
        if '.' in a:
            return int(a[ : a.find('.')])
        else:
            return int(a[ : a.find('_')]) - 0.5

    def _get_max_page_idx(self):
        upper = 1
        while self._is_page_availiable(upper):
            upper *= 2
        lower = upper // 2
        while lower + 1 < upper:
            mid = (lower + upper) // 2
            if self._is_page_availiable(mid):
                lower = mid
            else:
                upper = mid
        return lower

    def _estimate_num_urls_between(self, min_url_num, max_page_idx):
        lower, upper = 1, max_page_idx
        while lower < upper:
            mid = (lower + upper) // 2
            re_calc = False
            while True:
                urls = self._get_page_urls(mid)
                if urls:
                    break
                if mid == lower:
                    lower += 1
                    re_calc = True
                    break
                elif mid == upper:
                    upper -= 1
                    re_calc = True
                    break
                else:
                    mid = random.randint(lower, upper)
            if re_calc:
                continue
            if min_url_num < self._get_url_compare_num(urls[0]):
                upper = mid - 1
            elif self._get_url_compare_num(urls[-1]) < min_url_num:
                lower = mid + 1
            else:
                lower = upper = mid
        m = (max_page_idx - lower) * 20
        if max_page_idx - lower < 50:
            rng = range(lower + 1, max_page_idx + 1)
            m = sum(len(self._get_page_urls(i)) for i in rng)
        urls = self._get_page_urls(lower)
        n = len([a for a in urls if self._get_url_compare_num(a) > min_url_num])
        return n + m

    def _get_page_urls(self, page_idx):
        while True:
            request = requests.get(self._get_url(page_idx), headers=HEADERS)
            if request.status_code == 200:
                break
            time.sleep(0.1)
        page = bs4.BeautifulSoup(request.text, 'lxml')
        ret = []
        for chd in page.find(attrs={'class': 'r-list-container'}).children:
            try:
                c = chd.get('class')
                if c is not None:
                    if 'r-list-sep' in c:
                        break
                    elif 'r-ent' in c:
                        a = chd.find_all('a')
                        if a:
                            ret.append(a[0].get('href'))
            except Exception as _:
                pass
        return ret

    def _get_url(self, page_idx):
        return self._url_prefix + 'index%d.html' % page_idx

    def _is_page_availiable(self, page_idx):
        request = requests.get(self._get_url(page_idx), headers=HEADERS)
        return request.status_code == 200
