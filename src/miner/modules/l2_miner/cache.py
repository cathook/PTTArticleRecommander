import datetime
import threading
import time

from modules.l2_miner.board_listener import BoardCacheInterface
from modules.l2_miner.real_backend_interface import RealBackendInterface
from modules.utils import sleep_if
from modules.utils import UnimplementedMethodCalled


class StorageInterface(object):
    @property
    def max_id(self):
        raise UnimplementedMethodCalled()

    @property
    def newest_doc(self):
        raise UnimplementedMethodCalled()

    @property
    def oldest_doc(self):
        raise UnimplementedMethodCalled()

    @property
    def updated(self):
        raise UnimplementedMethodCalled()

    def add_doc(self, idid, url):
        raise UnimplementedMethodCalled()

    def update_after_time(self, last_time):
        raise UnimplementedMethodCalled()

    def get_doc_by_id(self, idid):
        raise UnimplementedMethodCalled()

    def get_doc_by_url(self, url):
        raise UnimplementedMethodCalled()


class _CacheUpdateNotifier(threading.Thread):
    _TIME_PERIOD = 10 * 60
    _MAX_BIT = 1024

    def __init__(self, callback):
        super(_CacheUpdateNotifier, self).__init__()

        self._callback = callback
        self._stop_flag = False

    def stop(self):
        self._stop_flag = True

    def run(self):
        while True:
            for a in range(1, self._MAX_BIT + 1):
                if not sleep_if(self._TIME_PERIOD, lambda: not self._stop_flag):
                    return
                self._callback(self._TIME_PERIOD * (a & -a))


class Cache(BoardCacheInterface, RealBackendInterface):
    def __init__(self, logger, cache_size, storage):
        self._logger = logger
        self._cache_size = cache_size
        self._storage = storage

        self._docs = []
        self._max_id = -1
        self._min_id = 0
        self._url_to_id = {}
        self._cache_lock = threading.Lock()

        self._update_thr = _CacheUpdateNotifier(self._update_nearest_docs)
        self._update_thr.start()

    def stop_auto_update(self):
        self._update_thr.stop()
        self._update_thr.join()

    @property
    def newest_doc(self):
        return self._storage.newest_doc

    @property
    def oldest_doc(self):
        return self._storage.oldest_doc

    def add_doc(self, idid, url):
        return self._storage.add_doc(idid, url)

    @property
    def max_id(self):
        return self._storage.max_id

    @property
    def updated(self):
        return self._storage.updated

    def yield_doc_meta_data_reversed_from_id(self, idid):
        while idid >= 0:
            doc = self._safe_get_doc_by_id(idid)
            yield doc.meta_data
            idid -= 1

    def get_doc_real_data(self, idid):
        return self._safe_get_doc_by_id(idid).real_data

    def get_url_by_id(self, idid):
        return self._safe_get_doc_by_id(idid).url

    def get_id_by_url(self, url):
        with self._cache_lock:
            if url in self._url_to_id:
                return self._url_to_id[url]

        doc = self._storage.get_doc_by_url(url)
        if doc is None:
            return -1
        self._cache_doc_if_ok(doc)
        return doc.meta_data.idid

    def _update_nearest_docs(self, time_delta):
        t = int(datetime.datetime.now().timestamp() - time_delta)
        (min_id, max_id) = self._storage.update_after_time(t)

        # Flushes the cache to force the later queries causing an cache update.
        self._logger.info('Flush the cached documents posted later than %d' % t)
        with self._cache_lock:
            for idid in range(max(self._min_id, min_id),
                              min(self._max_id, max_id) + 1):
                self._clean_cache_entry(idid - self._min_id)

    def _safe_get_doc_by_id(self, idid):
        # Looks up the cache first.
        doc = None
        with self._cache_lock:
            if self._min_id <= idid <= self._max_id:
                doc = self._docs[idid - self._min_id]

        # Fetches the document from the storage if it is not in the cache.
        if doc is None:
            self._logger.info('Cache missed at document %d' % idid)
            doc = self._storage.get_doc_by_id(idid)
            self._cache_doc_if_ok(doc)

        return doc

    def _cache_doc_if_ok(self, doc):
        idid = doc.meta_data.idid
        with self._cache_lock:
            if self._max_id < idid:
                # Always caches the newer, so it moves the cache forward.
                num_need_add = idid - self._max_id
                if num_need_add >= self._cache_size:
                    self._url_to_id = {}
                    self._docs = [None]
                    self._min_id = self._max_id = idid
                else:
                    self._shrink_cache_size(self._cache_size - num_need_add)
                    self._docs = self._docs + [None] * num_need_add
                    self._max_id = idid
            elif idid < self._min_id and \
                    (self._max_id - idid + 1) <= self._cache_size:
                self._docs = [None] * (self._min_id - idid) + self._docs
                self._min_id = idid

            if self._min_id <= idid <= self._max_id:
                self._logger.info('Cache the document %d, cache queue [%d, %d].'
                                  % (idid, self._min_id, self._max_id))
                self._docs[idid - self._min_id] = doc
                self._url_to_id[doc.url] = idid

    def _shrink_cache_size(self, size):
        while len(self._docs) > size:
            self._clean_cache_entry(0)
            self._docs.pop(0)
            self._min_id += 1

    def _clean_cache_entry(self, i):
        doc = self._docs[i]
        if doc is not None:
            del self._url_to_id[doc.url]
            self._docs[i] = None
