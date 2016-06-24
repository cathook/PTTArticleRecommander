import datetime
import inspect
import logging
import os
import sys
import tempfile
import threading
import time
import unittest

from modules.l2_miner import common
from modules.l2_miner import storage
from modules.protocol import types


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class _FakeCrewer(storage.CrewerInterface):
    def __init__(self):
        self.counter = 0

    def get_doc_by_url(self, url):
        self.counter += 1
        doc = common.ADocument()
        doc.url = url
        doc.meta_data = types.DocMetaData(
                None, None, url, url, int(url), url, [0, 0, 0])
        doc.real_data = types.DocRealData(url, [])
        return doc


class TestStorage(unittest.TestCase):
    def setUp(self):
        self.lg = logging.getLogger()
        self.fc = _FakeCrewer()
        self.tdir = tempfile.mkdtemp()
        self.storage = None

    def _initStorage(self):
        self.storage = storage.Storage(self.lg, self.tdir, self.fc, int)

    def runTest(self):
        self._initStorage()

        self.assertEqual(self.storage.max_id, -1)
        self.assertEqual(self.storage.newest_doc, None)
        self.assertEqual(self.storage.oldest_doc, None)
        self.assertEqual(self.storage.updated, False)
        self.assertEqual(self.storage.update_after_time(0), (-1, -1))

        self._test_add_doc(10, True)
        self.assertEqual(self.storage.updated, True)

        for i in range(5):
            self.assertEqual(self.storage.get_doc_by_id(10).url, '10')
        self.assertEqual(self.fc.counter, 1)

        self._test_add_doc(10, False)
        self._test_add_doc(7, True)

        thr = threading.Thread(target=self._add_from, args=(30, 12))
        thr.start()

        self.assertEqual(self.storage.get_doc_by_id(23).url, '23')
        self.assertEqual(self.storage.get_doc_by_id(21).url, '21')
        self.assertEqual(self.storage.get_doc_by_id(27).url, '27')
        self.assertEqual(self.storage.get_doc_by_id(27).url, '27')
        t0 = datetime.datetime.now().timestamp()
        self.assertEqual(self.storage.get_doc_by_id(10).url, '10')
        self.assertTrue((datetime.datetime.now().timestamp() - t0) < 0.05)

        thr.join()
        self.assertEqual(self.fc.counter, 21)

        self.fc.counter = 0
        self._initStorage()

        self.assertEqual(self.storage.max_id, 30)
        self.assertEqual(self.storage.newest_doc.url, '30')
        self.assertEqual(self.storage.oldest_doc.url, '12')
        self.assertEqual(self.storage.updated, False)
        self.assertEqual(self.fc.counter, 0)
        self.assertEqual(self.storage.update_after_time(13), (13, 30))
        self.assertEqual(self.fc.counter, 18)
        self.assertEqual(self.storage.update_after_time(6), (7, 30))
        self.assertEqual(self.fc.counter, 18 + 21)

    def _test_add_doc(self, idid, should_add):
        ct = self.fc.counter
        self.assertEqual(self.storage.add_doc(idid, str(idid)), should_add)
        self.assertEqual(self.fc.counter, ct + (1 if should_add else 0))

    def _add_from(self, max_id, min_id):
        for idid in range(max_id, min_id - 1, -1):
            self.storage.add_doc(idid, str(idid))
            time.sleep(0.1)
