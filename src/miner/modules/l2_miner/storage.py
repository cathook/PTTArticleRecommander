import os
import pickle
import sys
import threading

from modules.l2_miner.cache import StorageInterface
from modules.l2_miner.common import ADocument


class CrewerInterface(object):
    def get_doc_by_url(self, url):
        pass


class Storage(StorageInterface):
    def __init__(self, logger, data_dir, crewer, url_num_getter=None):
        self._logger = logger
        self._data_dir = data_dir
        self._crewer = crewer
        self.url_num_getter = url_num_getter
        self._updated = False
        self._stop_flag = False

        is_file = lambda f: os.path.isfile(os.path.join(data_dir, f))
        self._max_id = max(
                [-1] + [int(f) for f in os.listdir(data_dir) if is_file(f)])

        self._lock = threading.Condition()

    def notify_stop(self):
        self._stop_flag = True

    @property
    def max_id(self):
        with self._lock:
            return self._max_id

    @property
    def newest_doc(self):
        a = self.max_id
        return self.get_doc_by_id(a) if a >= 0 else None

    @property
    def oldest_doc(self):
        prev_fname = None
        for idid in range(self._max_id, -1, -1):
            fname = self._get_filename_by_id(idid)
            with self._lock:
                if not os.path.isfile(fname):
                    if prev_fname is None:
                        return None
                    with open(prev_fname, 'rb') as f:
                        return self._load_obj_from_file(f)
            prev_fname = fname
        return None

    @property
    def updated(self):
        return self._updated

    def add_doc(self, idid, url):
        if not os.path.isfile(self._get_filename_by_id(idid)):
            doc = self._update_doc(idid, url)
            self._updated = True
            return True
        return False

    def update_after_time(self, last_time):
        self._logger.info(
                'Start to update all documents posted after %d' % last_time)
        min_id = max_id = self.max_id
        for idid in range(max_id, -1, -1):
            fname = self._get_filename_by_id(idid)
            if not os.path.isfile(fname):
                continue
            doc = self.get_doc_by_id(idid)
            if doc.meta_data.post_time < last_time:
                self._logger.info('Post time of document %d is %d < %d'
                                  % (idid, doc.meta_data.post_time, last_time))
                break
            self._logger.info('Update doc %d' % doc.meta_data.idid)
            self._update_doc(doc.meta_data.idid, doc.url)
            min_id = idid

        self._logger.info(
                'Document [%d, %d] are re-fetched.' % (min_id, max_id))
        return (min_id, max_id)

    def get_doc_by_id(self, idid):
        fname = self._get_filename_by_id(idid)
        with self._lock:
            # Waits until the file is availiable.
            while not os.path.isfile(fname):
                self._logger.info(
                        'Document %r is currently unavailiable, wait.' % idid)
                self._lock.wait(1)
                if self._stop_flag:
                    return None

            with open(fname, 'rb') as f:
                self._logger.info('Load doc %d from the storage.' % idid)
                return self._load_obj_from_file(f)

    def get_doc_by_url(self, url):
        url_num = self.url_num_getter(url)
        for idid in range(self._max_id, -1, -1):
            fname = self._get_filename_by_id(idid)
            with self._lock:
                if not os.path.isfile(fname):
                    continue
                with open(fname, 'rb') as f:
                    doc = self._load_obj_from_file(f)
            url_num2 = self.url_num_getter(doc.url)
            if url_num == url_num2:
                return doc
            if url_num2 < url_num:
                break
        return None

    def _update_doc(self, idid, url):
        doc = self._crewer.get_doc_by_url(url)
        doc.meta_data.idid = idid
        doc.meta_data.prev_id = idid
        s = self._dump_obj_to_str(doc)
        with self._lock:
            self._logger.info('Dump doc %d to the storage.' % idid)
            with open(self._get_filename_by_id(idid), 'wb') as f:
                f.write(s)

            self._max_id = max(self._max_id, idid)

            # notifies the waiting guys that the storage data was updated
            self._lock.notify_all()
        return doc

    def _get_filename_by_id(self, idid):
        return os.path.join(self._data_dir, str(idid))

    def _dump_obj_to_str(self, obj):
        try:
            return pickle.dumps(obj)
        except RuntimeError as e:
            self._handle_pickle_re()
            raise

    def _load_obj_from_file(self, f):
        try:
            return pickle.load(f)
        except RuntimeError as e:
            self._handle_pickle_re()
            raise

    def _handle_pickle_re(self):
        curr_lim = sys.getrecursionlimit()
        self._logger.error('RE, increase limit from %d' % curr_lim)
        sys.setrecursionlimit(int(curr_lim * 1.5))
