import os
import threading
import time

from modules.backend_interface import BackendInterface
from modules.l2_miner.board_listener import BoardListener
from modules.l2_miner.cache import Cache
from modules.l2_miner.crewer import Crewer
from modules.l2_miner.storage import Storage
from modules.protocol.types import DocIdentity


class RealBackend(BackendInterface):
    def __init__(self, logger, cache_size, data_dir):
        self._logger = logger
        self._cache_size = cache_size
        self._data_dir = data_dir

        self._workers = {}
        self._worker_lock = threading.Lock()
        self._crewer = Crewer(self._logger.getChild('crewer'))

        self._safe_create_dir(self._data_dir)

    def get_max_id(self, board):
        return self._get_cache(board).max_id

    def get_doc_meta_data_after_id(self, board, idid):
        metas = self._yield_board_meta(board, self._get_cache(board).max_id)
        ret = []
        for meta in metas:
            if meta.idid < idid:
                break
            ret = [meta] + ret
        return ret

    def get_doc_meta_data_after_time(self, board, post_time):
        metas = self._yield_board_meta(board, self._get_cache(board).max_id)
        ret = []
        for meta in metas:
            if meta.post_time < post_time:
                break
            ret = [meta] + ret
        return ret

    def get_doc_meta_data_of_author(self, board, author):
        metas = self._yield_board_meta(board, self._get_cache(board).max_id)
        return reversed([m for m in metas if m.author == author])

    def get_doc_meta_data_series(self, board, idid):
        for meta in self._yield_board_meta(board, idid):
            return [meta]

    def get_doc_real_data(self, board, idid):
        return self._get_cache(board).get_doc_real_data(idid)

    def get_id_by_url(self, url):
        for board in self._workers:
            if board in url:
                idid = self._get_cache(board).get_id_by_url(url)
                return DocIdentity(board, idid)
        return DocIdentity('', -1)

    def get_url_by_id(self, board, idid):
        return self._get_cache(board).get_url_by_id(idid)

    def destroy(self):
        with self._worker_lock:
            self._logger.info('Destroy.')
            for (c, b, s, d) in self._workers.values():
                b.stop()
                b.join()
                c.stop_auto_update()
                s.notify_stop()
            self._workers = {}

    def _yield_board_meta(self, board, idid):
        cache = self._get_cache(board)
        for meta_data in cache.yield_doc_meta_data_reversed_from_id(idid):
            meta_data.board = board
            yield meta_data

    def _get_cache(self, board):
        with self._worker_lock:
            if board not in self._workers:
                self._logger.info(
                        'First query about board %r, create worker.' % board)
                d = os.path.join(self._data_dir, board)
                self._safe_create_dir(d)
                gl = lambda x: self._logger.getChild('%s[%s]' % (x, board))
                s = Storage(gl('Storage'), d, self._crewer)
                c = Cache(gl('Cache'), self._cache_size, s)
                b = BoardListener(gl('BoardListener'), board, 5 * 60, c)
                s.url_num_getter = b.get_url_compare_num
                b.start()
                self._workers[board] = (c, b, s, d)
                while not c.updated:
                    time.sleep(0.1)
            return self._workers[board][0]

    @staticmethod
    def _safe_create_dir(d):
        try:
            os.makedirs(d, mode=0o755)
        except Exception as _:
            pass
