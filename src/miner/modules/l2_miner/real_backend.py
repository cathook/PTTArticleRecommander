import os
import time

from modules.backend_interface import BackendInterface
from modules.l2_miner.board_listener import BoardListener
from modules.l2_miner.cache import Cache
from modules.l2_miner.crewer import Crewer
from modules.l2_miner.storage import Storage


class RealBackend(BackendInterface):
    def __init__(self, logger, cache_size, data_dir):
        self._logger = logger
        self._cache_size = cache_size
        self._data_dir = data_dir

        try:
            os.makedirs(self._data_dir, 0o755)
        except Exception as _:
            pass

        self._workers = {}
        self._crewer = Crewer(self._logger.getChild('crewer'))

    def get_max_id(self, board):
        return self._get_cache(board).max_id

    def get_doc_meta_data_after_id(self, board, idid):
        metas = self._yield_board_docs(board, self._get_cache(board).max_id)
        ret = []
        for meta in metas:
            if meta.idid < idid:
                break
            ret = [meta] + ret
        return ret

    def get_doc_meta_data_after_time(self, board, post_time):
        metas = self._yield_board_docs(board, self._get_cache(board).max_id)
        ret = []
        for meta in metas:
            if meta.post_time < post_time:
                break
            ret = [meta] + ret
        return ret

    def get_doc_meta_data_of_author(self, board, author):
        metas = self._yield_board_docs(board, self._get_cache(board).max_id)
        return reversed([m for m in metas if m.author == author])

    def get_doc_meta_data_series(self, board, idid):
        for meta in self._yield_board_docs(board, idid):
            return [meta]

    def get_doc_real_data(self, board, idid):
        return self._get_cache(board).get_doc_real_data(idid)

    def get_id_by_url(self, url):
        return self._get_cache(board).get_id_by_url(url)

    def get_url_by_id(self, board, idid):
        return self._get_cache(board).get_url_by_id(idid)

    def _yield_board_docs(self, board, idid):
        cache = self._get_cache(board)
        for meta_data in cache.yield_doc_meta_data_reversed_from_id(idid):
            meta_data.board = board
            yield meta_data

    def _get_cache(self, board):
        if board not in self._workers:
            self._logger.info(
                    'First query about board %r, create worker.' % board)
            d = os.path.join(self._data_dir, board)
            try:
                os.mkdir(d, mode=0o755)
            except Exception as _:
                pass
            gl = lambda x: self._logger.getChild('%s[%s]' % (x, board))
            s = Storage(gl('Storage'), d, self._crewer)
            c = Cache(gl('Cache'), self._cache_size, s)
            b = BoardListener(gl('BoardListener'), board, 10 * 60, c)
            b.start()
            self._workers[board] = (c, b, s, d)
            while not c.updated:
                time.sleep(0.1)
        return self._workers[board][0]
