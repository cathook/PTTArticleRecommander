import inspect
import sys
import threading
import time
import unittest


from modules.l2_miner import cache
from modules.l2_miner import common
from modules.protocol import types


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class _FakeStorage(cache.StorageInterface):
    def __init__(self):
        self.num_docs = 0
        self.num_called = 0

    @property
    def max_id(self):
        return self.num_docs - 1

    @property
    def newest_url(self):
        return str(self.num_docs - 1)

    def add_url(self, idid, url):
        time.sleep(0.1)
        self.num_docs += 1

    def update_after_time(self, last_time):
        pass

    def yield_doc_reverse_from_id(self, idid):
        while idid >= 0:
            yield self.get_doc_by_id(idid)

    def get_doc_by_id(self, idid):
        self.num_called += 1
        doc = common.ADocument()
        doc.url = str(idid)
        doc.meta_data = types.DocMetaData(
                idid, idid, 'meow', 'wang', 0, 'board', [0, 0, 0])
        doc.real_data = types.DocRealData('walala', [])
        return doc

    def get_id_by_url(self, url):
        return int(url)

    def get_url_by_id(self, idid):
        return str(idid)


class TestCache1(unittest.TestCase):
    def runTest(self):
        fs = _FakeStorage()
        c = cache.Cache(5, fs)
        self.assertEqual(c.newest_url, '-1')
        def func():
            c.add_urls(8, ['7', '6', '5', '4', '3', '2', '1', '0'])
        thr = threading.Thread(target=func)
        thr.start()
        time.sleep(0.1)
        self.assertEqual(c.max_id, 7)
        idid = 7
        for meta in c.yield_doc_meta_data_reversed_from_id(idid):
            self.assertEqual(meta.idid, idid)
            idid -= 1
            if idid >= 2:
                break
        fs.num_called = 0
        self.assertEqual(c.get_doc_real_data(1).content, 'walala')
        self.assertEqual(c.get_id_by_url('1'), 1)
        self.assertEqual(c.get_url_by_id(3), '3')
        thr.join()
        self.assertEqual(fs.num_docs, 8)
        self.assertEqual(fs.num_called, 2)


class TestCache2(unittest.TestCase):
    def runTest(self):
        pass
