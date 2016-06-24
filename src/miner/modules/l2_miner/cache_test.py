import inspect
import logging
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
        self.num_called = 0
        self.last_time = -1

    @property
    def max_id(self):
        return 12345

    @property
    def newest_doc(self):
        return 'new_meow'

    @property
    def oldest_doc(self):
        return 'old_meow'

    @property
    def updated(self):
        return True

    def add_doc(self, idid, url):
        self.idid = idid
        return idid % 2 == 0

    def update_after_time(self, last_time):
        self.last_time = last_time
        return 0, 9999

    def get_doc_by_id(self, idid):
        doc = common.ADocument()
        doc.url = str(idid)
        doc.meta_data = types.DocMetaData(
                idid, idid, 'meow', 'wang', idid, None, [0, 0, 0])
        doc.real_data = types.DocRealData(str(self.last_time), [])
        self.num_called += 1
        return doc

    def get_doc_by_url(self, url):
        doc = common.ADocument()
        doc.url = url
        doc.meta_data = types.DocMetaData(
                int(url), int(url), 'meow', 'wang', int(url), None, [0, 0, 0])
        doc.real_data = types.DocRealData(str(self.last_time), [])
        self.num_called += 1
        return doc


class TestCache(unittest.TestCase):
    def runTest(self):
        fs = _FakeStorage()
        c = cache.Cache(logging.getLogger(), 5, fs)
        self.assertEqual(c.max_id, 12345)
        self.assertEqual(c.newest_doc, 'new_meow')
        self.assertEqual(c.oldest_doc, 'old_meow')
        self.assertEqual(c.add_doc(123, '123'), False)
        self.assertEqual(c.add_doc(124, '124'), True)
        i = 10
        for doc in c.yield_doc_meta_data_reversed_from_id(10):
            self.assertEqual(doc.idid, i)
            i -= 1
        self.assertEqual(fs.num_called, 11)
        self.assertEqual(c.get_doc_real_data(9).content, '-1')
        self.assertEqual(c.get_doc_real_data(7).content, '-1')
        self.assertEqual(c.get_url_by_id(7), '7')
        self.assertEqual(fs.num_called, 11)
        self.assertEqual(c.get_doc_real_data(3).content, '-1')
        self.assertEqual(fs.num_called, 12)
        self.assertEqual(c.get_doc_real_data(12).content, '-1')
        self.assertEqual(c.get_doc_real_data(10).content, '-1')
        self.assertEqual(c.get_url_by_id(10), '10')
        self.assertEqual(fs.num_called, 13)
        self.assertEqual(c.get_doc_real_data(7).content, '-1')
        self.assertEqual(c.get_doc_real_data(7).content, '-1')
        self.assertEqual(fs.num_called, 15)
        self.assertEqual(c.get_doc_real_data(18).content, '-1')
        self.assertEqual(c.get_doc_real_data(18).content, '-1')
        self.assertEqual(c.get_doc_real_data(18).content, '-1')
        self.assertEqual(fs.num_called, 16)
        self.assertEqual(c.get_doc_real_data(17).content, '-1')
        self.assertEqual(fs.num_called, 17)
        self.assertEqual(c.get_url_by_id(17), '17')
        self.assertEqual(c.get_url_by_id(18), '18')
        self.assertEqual(c.get_url_by_id(12), '12')
        self.assertEqual(fs.num_called, 18)
        for i in range(14, 19):
            self.assertEqual(c.get_url_by_id(i), str(i))
        self.assertEqual(fs.num_called, 21)
        c._update_nearest_docs(123)
        self.assertEqual(c.get_url_by_id(18), '18')
        self.assertEqual(fs.num_called, 22)
        c.stop_auto_update()
