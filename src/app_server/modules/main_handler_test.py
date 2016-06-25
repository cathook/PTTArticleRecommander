import inspect
import logging
import socket
import sys
import threading
import time
import unittest

from modules import main_handler
from modules.protocol import types


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class _FakeMiner(object):
    def get_url_by_doc_identity(self, iden):
        return iden.board + '%d' % iden.idid

    def get_doc_identity_by_url(self, url):
        return types.DocIdentity(url, 1)


class _FakeArticleAnalyst(object):
    def get_doc_rel_info(self, idid):
        return types.DocRelInfo([idid], [idid, idid], [idid, idid, idid])


class TestMainHandler(unittest.TestCase):
    def runTest(self):
        a = main_handler.MainHandler(
                _FakeMiner(), _FakeArticleAnalyst(), logging.getLogger())
        b = a.handle_json({'url': 'aabbccdd'})
        self.assertEqual(len(b), 3)
        for i in range(3):
            key = ['positive', 'negative', 'neutral'][i]
            self.assertEqual(len(b[key]), i + 1)
            for x in b[key]:
                self.assertEqual(x['url'], 'aabbccdd1')
