import inspect
import logging
import sys
import tempfile
import unittest

from modules.l2_miner import real_backend


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class TestRealBackend(unittest.TestCase):
    def runTest(self):
        pass
