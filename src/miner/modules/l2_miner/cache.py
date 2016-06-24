import datetime
import threading
import time

from modules.l2_miner.real_backend_interface import RealBackendInterface
from modules.l2_miner.board_listener import BoardCacheInterface


class StorageInterface(object):
    @property
    def max_id(self):
        pass

    @property
    def newest_url(self):
        pass

    def add_url(self, idid, url):
        pass

    def update_after_time(self, last_time):
        pass

    def yield_doc_reverse_from_id(self, idid):
        pass

    def yield_stored_doc_reverse_from_id(self, idid):
        pass

    def get_doc_by_id(self, idid):
        pass

    def get_id_by_url(self, url):
        pass

    def get_url_by_id(self, idid):
        pass


class _CacheUpdateNotifier(threading.Thread):
    _TIME_PERIOD = 60 * 10

    def __init__(self, callback):
        super(_CacheUpdateNotifier, self).__init__()
        self.daemon = True

        self._callback = callback

    def run(self):
        ct = 0
        while True:
            time.sleep(self._TIME_PERIOD)
            ct += 1
            i = 1
            while (ct & i) == 0:
                i *= 2
            self._callback(self._TIME_PERIOD * i)
            if i == 1024:
                ct = 0


class Cache(BoardCacheInterface, RealBackendInterface):
    def __init__(self, logger, cache_size, storage):
        self._logger = logger
        self._cache_size = cache_size
        self._storage = storage

        self._cache = []
        self._max_id = 0
        self._min_id = 0
        self._url_to_id = {}
        self._cache_lock = threading.Lock()
        self.updated = False

        self._init_from_storage()

        self._update_thr = _CacheUpdateNotifier(self.update)
        self._update_thr.start()

    @property
    def newest_url(self):
        with self._cache_lock:
            if self._cache and self._cache[-1] is not None:
                return self._cache[-1].url
        return self._storage.newest_url

    @property
    def max_id(self):
        with self._cache_lock:
            return self._max_id

    def update(self, post_time):
        t = int(datetime.datetime.now().timestamp() - post_time)
        self._logger.info(
                'Start to update docs which was posted later than %d' % t)
        self._storage.update_after_time(t)
        with self._cache_lock:
            for i in range(len(self._cache)):
                if self._cache[i] is not None and \
                        self._cache[i].meta_data.post_time >= t:
                    self._cache[i] = None

    def add_urls(self, num_urls, urls_gen):
        with self._cache_lock:
            self._max_id += num_urls
            curr_id = self._max_id
            if num_urls <= self._cache_size:
                self._cache = self._cache + [None] * num_urls
                self._ensure_cache_in_size()
            else:
                self._cache = [None] * self._cache_size
                self._min_id = self._max_id - self._cache_size + 1
        self.updated = True

        ct = 0
        for url in urls_gen:
            doc = self._storage.add_url(curr_id, url)
            with self._cache_lock:
                if self._min_id <= curr_id:
                    self._cache[curr_id - self._min_id] = doc
                    self._logger.info('Cached doc %d' % curr_id)
            curr_id -= 1
            ct += 1
            if ct >= num_urls:
                break

    def yield_doc_meta_data_reversed_from_id(self, idid):
        while idid >= 0:
            doc = self._safe_get_doc_by_id(idid)
            yield doc.meta_data
            idid -= 1

    def get_doc_real_data(self, idid):
        doc = self._safe_get_doc_by_id(idid)
        return doc.real_data

    def get_url_by_id(self, idid):
        doc = self._safe_get_doc_by_id(idid)
        return doc.url

    def get_id_by_url(self, url):
        with self._cache_lock:
            if url in self._url_to_id:
                return self._url_to_id[url]
        return self._storage.get_id_by_url(url)

    def _init_from_storage(self):
        with self._cache_lock:
            self._max_id = self._storage.max_id
            y = self._storage.yield_stored_doc_reverse_from_id(self._max_id)
            for doc in y:
                self._cache = [doc] + self._cache
                self._url_to_id[doc.url] = doc.meta_data.idid;
                if len(self._cache) >= self._cache_size:
                    break
            self._min_id = self._cache[0].meta_data.idid if self._cache else 0

    def _ensure_cache_in_size(self):
        while len(self._cache) > self._cache_size:
            doc = self._cache[0]
            self._cache = self._cache[1 : ]
            del self._url_to_id[doc.url]
            self._min_id += 1

    def _safe_get_doc_by_id(self, idid):
        doc = None
        with self._cache_lock:
            if self._min_id <= idid:
                doc = self._cache[idid - self._min_id]
        if doc is None:
            doc = self._storage.get_doc_by_id(idid)
        return doc
