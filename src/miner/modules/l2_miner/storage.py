import os
import pickle
import threading

from modules.l2_miner.cache import StorageInterface
from modules.l2_miner.common import ADocument


class CrewerInterface(object):
    def get_doc_by_url(self, url):
        pass


class Storage(StorageInterface):
    def __init__(self, logger, data_dir, crewer):
        self._logger = logger
        self._data_dir = data_dir
        self._crewer = crewer

        fs = [os.path.join(data_dir, f) for f in os.listdir(data_dir)]
        self._max_id = max(
                [-1] + [int(os.path.basename(f))
                        for f in fs if os.path.isfile(f)])

        self._lock = threading.Condition()

    @property
    def max_id(self):
        with self._lock:
            return self._max_id

    @property
    def newest_url(self):
        a = self.max_id
        return self.get_doc_by_id(a).url if a >= 0 else None

    def add_url(self, idid, url):
        fname = os.path.join(self._data_dir, str(idid))
        if not os.path.isfile(fname):
            self._update_url(idid, url)

    def update_after_time(self, last_time):
        self._logger.info(
                'Start to update all documents posted after %d' % last_time)
        print('max=%r' % self._max_id)
        for idid in range(self._max_id, -1, -1):
            print('aaaaaaaaaaaa')
            doc = self.get_doc_by_id(idid)
            print('bbbbbbbbb')
            if doc.meta_data.post_time < last_time:
                break
            self._logger.info('Update doc %d' % doc.meta_data.idid)
            self._update_url(doc.meta_data.idid, doc.url)
        self._logger.info('End update')

    def yield_doc_reverse_from_id(self, idid):
        for i in range(idid, -1, -1):
            yield self.get_doc_by_id(i)

    def yield_stored_doc_reverse_from_id(self, idid):
        for i in range(idid, -1, -1):
            fname = os.path.join(self._data_dir, str(i))
            with self._lock:
                if not os.path.isfile(fname):
                    break
                with open(fname, 'rb') as f:
                    self._logger.info('Load doc %d from the storage.' % idid)
                    yield pickle.load(f)

    def get_doc_by_id(self, idid):
        fname = os.path.join(self._data_dir, str(idid))
        with self._lock:
            while not os.path.isfile(fname):
                self._logger.info('Document %r is currently unavailiable, wait.'
                                  % idid)
                self._lock.wait()
            with open(fname, 'rb') as f:
                self._logger.info('Load doc %d from the storage.' % idid)
                return pickle.load(f)

    def get_id_by_url(self, url):
        lower, upper = 0, self.max_id
        while lower <= upper:
            mid = (lower + upper) // 2
            doc = self.get_doc_by_id(mid)
            if doc.url < url:
                lower = mid + 1
            elif doc.url > url:
                upper = mid - 1
            else:
                return doc.meta_data.idid
        return -1

    def get_url_by_id(self, idid):
        if 0 <= idid <= self._max_id:
            return self.get_doc_by_id(idid).url
        else:
            return ''

    def _update_url(self, idid, url):
        doc = self._crewer.get_doc_by_url(url)
        doc.meta_data.idid = idid
        doc.meta_data.prev_id = idid
        fname = os.path.join(self._data_dir, str(idid))
        with self._lock:
            self._logger.info('Dump doc %d to the storage.' % idid)
            s = pickle.dumps(doc)
            with open(fname, 'wb') as f:
                f.write(s)

            self._max_id = max(self._max_id, idid)
            self._logger.info('An document was added/updated, id=%r' % idid)

            # notifies the waiting guys that the storage data was updated
            self._lock.notify()
