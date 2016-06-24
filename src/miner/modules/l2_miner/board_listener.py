import bs4
import logging
import random
import requests
import threading
import time

from modules.l2_miner.common import HEADERS
from modules.l2_miner.common import SERVER_ADDR
from modules.l2_miner.common import URL_PREFIX
from modules.utils import UnimplementedMethodCalled
from modules.utils import get_exception_msg
from modules.utils import sleep_if


class BoardCacheInterface(object):
    @property
    def newest_doc(self):
        raise UnimplementedMethodCalled()

    @property
    def oldest_doc(self):
        raise UnimplementedMethodCalled()

    def add_doc(self, idid, url):
        raise UnimplementedMethodCalled()


class _UrlGenerator(object):
    def __init__(self,
                 urls_getter, url_num_getter,
                 min_url_num, max_page_idx, urls, prev_url):
        self._urls_getter = urls_getter
        self._url_num_getter = url_num_getter
        self._min_url_num = min_url_num
        self._curr_page_idx = max_page_idx
        self._curr_page_urls = urls
        self._prev_url = prev_url

    def __iter__(self):
        return self

    def __next__(self):
        while self._curr_page_idx > 1 and not self._curr_page_urls:
            self._curr_page_idx -= 1
            self._curr_page_urls = self._urls_getter(self._curr_page_idx)
        if not self._curr_page_urls:
            return self._prev_url[ : -len('.X.XXX.html')] + '_x'

        curr = self._curr_page_urls.pop()
        if self._url_num_getter(curr) <= self._min_url_num:
            return self._prev_url[ : -len('.X.XXX.html')] + '_x'

        self._prev_url = SERVER_ADDR + curr
        return SERVER_ADDR + curr


class _UpdateThread(threading.Thread):
    def __init__(self, logger, died_callback, cache, max_id, url_generator):
        super(_UpdateThread, self).__init__()

        self._logger = logger
        self._died_callback = died_callback

        self._cache = cache
        self._curr_id = max_id
        self._url_generator = url_generator

        self._stop_flag = False

    def run(self):
        try:
            # Iterately asks the cache to add an document from new to old.
            for url in self._url_generator:
                succ = self._cache.add_doc(self._curr_id, url)
                self._curr_id -= 1

                if not succ or self._curr_id < 0 or self._stop_flag:
                    break
        except Exception as e:
            self._logger.warning(get_exception_msg(e))
            self._died_callback(self)

    def stop(self):
        self._stop_flag = True


class BoardListener(threading.Thread):
    def __init__(self, logger, board_name, update_timestamp, cache):
        super(BoardListener, self).__init__()

        self._logger = logger
        self._board_name = board_name
        self._url_prefix = URL_PREFIX + board_name + '/'
        self._update_timestamp = update_timestamp
        self._cache = cache

        self._stop_flag = False
        self._update_threads = []
        self._died_threads = []
        self._sync_lock = threading.Lock()

        logging.getLogger('urllib3').setLevel(logging.WARNING)

    def run(self):
        while not self._stop_flag:
            try:
                newest_doc = self._cache.newest_doc
                oldest_doc = self._cache.oldest_doc
                max_page_idx = self._get_max_page_idx()

                self._run_fetch_new_docs(newest_doc, max_page_idx)
                self._run_fill_old_docs(oldest_doc, max_page_idx)
            except Exception as e:
                self._logger.warn(get_exception_msg(e))

            if not sleep_if(self._update_timestamp,
                            self._clean_and_check_should_continue):
                break

    def stop(self):
        with self._sync_lock:
            self._stop_flag = True
        for thr in self._update_threads:
            thr.stop()
            thr.join()
        self._update_threads = []
        with self._sync_lock:
            self._clean_died_threads()

    def get_url_compare_num(self, url):
        if url is None:
            return 0
        a = url[url.find(self._board_name) + len(self._board_name) + 3: ]
        if '.' in a:
            return int(a[ : a.find('.')])
        else:
            return int(a[ : a.find('_')]) - 0.5

    def _run_fetch_new_docs(self, newest_doc, max_page_idx):
        if newest_doc:
            url_num = self.get_url_compare_num(newest_doc.url)
            idid = newest_doc.meta_data.idid
        else:
            url_num = idid = -1

        self._logger.info('Estimates the number of unfetched documents.')
        n = self._estimate_num_urls_between(url_num, max_page_idx)
        if n > 0:
            self._logger.info(
                'Found about %d new docs, update the cache.' % n)
            urls = self._get_page_urls(max_page_idx)
            self._start_update_thread(idid + n, url_num, max_page_idx, urls)
        else:
            self._logger.info('Great, there is no new document on the server.')

    def _run_fill_old_docs(self, oldest_doc, max_page_idx):
        if oldest_doc:
            idid = oldest_doc.meta_data.idid
            self._logger.info('Fetch document with id less than %d.' % idid)
            url_num = self.get_url_compare_num(oldest_doc.url)
            (p, us) = self._get_page_contain_url_num(url_num, max_page_idx)
            us = [u for u in us if self.get_url_compare_num(u) < url_num]
            self._start_update_thread(
                    idid - 1, -1, max_page_idx, us, oldest_doc.url)
        else:
            self._logger.info('Great, the mirror is complete.')

    def _clean_and_check_should_continue(self):
        with self._sync_lock:
            if self._stop_flag:
                return False
            else:
                self._clean_died_threads()
                return True

    def _clean_died_threads(self):
        for thr in self._died_threads:
            thr.join()
        self._died_threads = []

    def _start_update_thread(
                self, max_id, min_url_num, max_page_idx, urls, prev_url=''):
        with self._sync_lock:
            if not self._stop_flag:
                url_generator = _UrlGenerator(self._get_page_urls,
                                              self.get_url_compare_num,
                                              min_url_num, max_page_idx,
                                              urls, prev_url)
                thr = _UpdateThread(self._logger.getChild('UpdateDaemon'),
                                    self._update_thread_died_callback,
                                    self._cache, max_id, url_generator)
                self._update_threads.append(thr)
                thr.start()

    def _update_thread_died_callback(self, thr):
        with self._sync_lock:
            if not self._stop_flag:
                self._died_threads.append(thr)
                self._update_threads.remove(thr)

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
        self._logger.info('There are %d pages.' % lower)
        return lower

    def _estimate_num_urls_between(self, min_url_num, max_page_idx):
        (lower, us) = self._get_page_contain_url_num(min_url_num, max_page_idx)
        if max_page_idx - lower > 50:
            m = (max_page_idx - lower) * 20
        else:
            # If the number of pages after the page which contains min_url_num
            # is not too large, we can estimate the number of urls tightly.
            m = sum(len(self._get_page_urls(i))
                    for i in range(lower + 1, max_page_idx + 1))
        n = len([a for a in us if self.get_url_compare_num(a) > min_url_num])
        return n + m

    def _get_page_contain_url_num(self, url_num, max_page_idx):
        lower, upper = 1, max_page_idx
        while lower < upper:
            mid = (lower + upper) // 2
            re_calc = False
            # We need to re-fetch another page's urls if all the documents in
            # the `mid`-th page are deleted.
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
            if url_num < self.get_url_compare_num(urls[0]):
                upper = mid - 1
            elif self.get_url_compare_num(urls[-1]) < url_num:
                lower = mid + 1
            else:
                lower = upper = mid
        return (lower, self._get_page_urls(lower))

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

    def _is_page_availiable(self, page_idx):
        for i in range(10):
            request = requests.get(self._get_url(page_idx), headers=HEADERS)
            if request.status_code == 200:
                return True
            time.sleep(0.1)
        return False

    def _get_url(self, page_idx):
        return self._url_prefix + 'index%d.html' % page_idx
