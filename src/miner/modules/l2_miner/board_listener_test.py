import inspect
import sys
import time
import unittest


from modules.l2_miner import board_listener


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class _FakeBoardCache(board_listener.BoardCacheInterface):
    def __init__(self):
        self.done_flag = False

    @property
    def newest_url(self):
        url = 'https://www.ptt.cc/bbs/Gossiping/M.1466641037.A.A80.html'
        print('my=%r' % url)
        return url

    def add_urls(self, num_urls, url_gen):
        print('n=%r' % num_urls)
        ct = 0
        for url in url_gen:
            ct += 1
            if ct >= 10:
                break
            print('url=%r' % url)
        self.done_flag = True


class TestBoardListener(unittest.TestCase):
    def runTest(self):
        c = _FakeBoardCache()
        bl = board_listener.BoardListener('Gossiping', 1000, c)
        bl.start()
        while not c.done_flag:
            time.sleep(0.01)
        bl.stop()
        self.assertTrue(True)
