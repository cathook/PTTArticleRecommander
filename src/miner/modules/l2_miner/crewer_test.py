import inspect
import logging
import sys
import unittest

from modules.l2_miner import crewer


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class _TestCrewerBase(unittest.TestCase):
    def setUp(self):
        self.crewer = crewer.Crewer(logging.getLogger())


class TestCrewerOkey(_TestCrewerBase):
    def runTest(self):
        url = 'https://www.ptt.cc/bbs/Gossiping/M.1455624927.A.11D.html'
        for i in range(100):
            doc = self.crewer.get_doc_by_url(url)
            self.assertEqual(doc.url, url)
            self.assertEqual(doc.meta_data.author, 'seabox')
            self.assertTrue(doc.meta_data.title.startswith('[公告] 八卦板板規'))
            self.assertTrue('第一條' in doc.real_data.content)

class TestCrewerBad(_TestCrewerBase):
    def runTest(self):
        urls = [
            'https://sadfdsafdsafdsafdsafs',
            'asdfasdfasdf',
            'https://www.google.com.tw',
            'https://www.ptt.cc/bbs/Gossiping/M.1455624927.X.XXX.html',
            'https://www.ptt.cc/bbs/Gossiping/M.1455624927_X.XXX.html'
        ]
        for url in urls:
            doc = self.crewer.get_doc_by_url(url)
            self.assertEqual(doc.meta_data.title, '')
