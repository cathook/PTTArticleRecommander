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
                None, None, url, url, 0, url, [0, 0, 0])
        doc.real_data = types.DocRealData(url, [])
        return doc

class TestCrewer(unittest.TestCase):
    def runTest(self):
        lg = logging.getLogger()
        fc = _FakeCrewer()
        tdir = tempfile.mkdtemp()
        s = storage.Storage(lg, tdir, fc)

        self.assertEqual(s.max_id, -1)
        self.assertEqual(s.newest_url, None)
        self.assertEqual(s.get_id_by_url('sadfasdf'), -1)
        self.assertEqual(s.get_url_by_id(123), '')

        s.add_url( 9, 'url09')
        s.add_url(10, 'url10')
        self.assertEqual(s.get_doc_by_id(9).url, 'url09')
        self.assertEqual(s.get_doc_by_id(9).real_data.content, 'url09')

        def func():
            for idid in range(8, -1, -1):
                s.add_url(idid, 'url%02d' % idid)
                time.sleep(0.3)
        thr = threading.Thread(target=func)
        thr.start()
        self.assertEqual(s.get_doc_by_id(6).url, 'url06')
        self.assertEqual(s.get_doc_by_id(7).url, 'url07')
        self.assertEqual(s.get_url_by_id(5), 'url05')
        self.assertEqual(s.get_id_by_url('url01'), 1)
        thr.join()

        fc.counter = 0
        s = storage.Storage(lg, tdir, fc)
        self.assertEqual(s.max_id, 10)
        self.assertEqual(s.get_doc_by_id(9).url, 'url09')
        self.assertEqual(s.get_doc_by_id(9).real_data.content, 'url09')
        self.assertEqual(s.get_doc_by_id(6).url, 'url06')
        self.assertEqual(s.get_doc_by_id(7).url, 'url07')
        self.assertEqual(s.get_url_by_id(5), 'url05')
        self.assertEqual(s.get_id_by_url('url01'), 1)
        self.assertEqual(fc.counter, 0)
