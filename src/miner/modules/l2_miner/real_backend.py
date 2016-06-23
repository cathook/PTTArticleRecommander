import os

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
            os.mkdirs(self._data_dir, 0o755)
        except Exception as _:
            pass

        self._workers = {}
        self._crewer = Crewer()

    def get_max_id(self, board):
        return self._get_cache(board).max_id

    def get_doc_meta_data_after_id(self, board, idid):
        docs = self._yield_board_docs(board, self._get_cache(board).max_id)
        ret = []
        for doc in docs:
            if doc.meta_data.idid < idid:
                break
            ret = [doc] + ret
        return ret

    def get_doc_meta_data_after_time(self, board, post_time):
        docs = self._yield_board_docs(board, self._get_cache(board).max_id)
        ret = []
        for doc in docs:
            if doc.meta_data.post_time < post_time:
                break
            ret = [doc] + ret
        return ret

    def get_doc_meta_data_of_author(self, board, author):
        docs = self._yield_board_docs(board, self._get_cache(board).max_id)
        return reversed([doc for doc in docs if doc.meta_data.author == author])

    def get_doc_meta_data_series(self, board, idid):
        for doc in self._yield_board_docs(board, idid):
            return [doc]

    def get_doc_real_data(self, board, idid):
        return self._get_cache(board).get_doc_real_data(idid)

    def get_id_by_url(self, url):
        return self._get_cache(board).get_id_by_url(url)

    def get_url_by_id(self, board, idid):
        return self._get_cache(board).get_url_by_id(idid)

    def _yield_board_docs(self, board, idid):
        cache = self._get_cache(board)
        for doc in cache.yield_doc_meta_data_reversed_from_id(idid):
            doc.meta_data.board = board
            yield doc

    def _get_cache(self, board):
        if board not in self._workers:
            d = os.path.join(self._data_dir, board)
            try:
                os.mkdir(d, mode=0o755)
            except Exception as _:
                pass
            s = Storage(d, self._crewer)
            c = Cache(self._cache_size, s)
            b = BoardListener(board, 10 * 60, c)
            self._workers[board] = (c, b, s, d)
        return self._workers[board][0]
