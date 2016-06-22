import inspect
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


class TestCrewer(unittest.TestCase):
    def runTest(self):
        a = crewer.Crewer('Gossiping')
        url = 'https://www.ptt.cc/bbs/Gossiping/M.1455624927.A.11D.html'
        doc = a.get_document_by_url(url)
        self.assertEqual(doc.url, url)
        self.assertEqual(doc.meta_data.author, 'seabox')
        self.assertTrue(doc.meta_data.title.startswith('[公告] 八卦板板規'))
        self.assertTrue('第一條' in doc.real_data.content)

        booo = False
        try:
            a.get_document_by_url('sadfdsafdsafdsafdsafs')
        except crewer.NotFoundException as e:
            booo = True
        self.assertTrue(booo)
