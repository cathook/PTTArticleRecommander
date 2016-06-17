from http.server import HTTPServer
import inspect
import json
import logging
import random
import requests
import sys
import threading
import time
import unittest

from modules import http_request_handler


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class _FakeMainHandler(object):
    def __init__(self, *args, **kwargs):
        pass

    def handle_json(self, obj):
        return obj


class TestHTTPRequestHandler(unittest.TestCase):
    def setUp(self):
        self._port = random.randint(8000, 9000)
        def handler(*args, **kwargs):
            return http_request_handler.HTTPRequestHandler(
                    _FakeMainHandler(), logging.getLogger(), *args, **kwargs)
        self._server = HTTPServer(('localhost', self._port), handler)
        self._thr = threading.Thread(target=self._server.serve_forever)
        self._thr.start()
        time.sleep(1)

    def runTest(self):
        d = {'key1': 'va中文1', 'key2': 'value2', 'key3': ['meow', 'wang']}
        r = requests.post('http://localhost:%d' % self._port,
                          data=json.dumps(d),
                          headers={'content-type': 'application/json'})
        self.assertEqual(r.status_code, 200)
        d2 = json.loads(r.text)
        self.assertEqual(d['key1'], d2['key1'])
        self.assertEqual(d['key2'], d2['key2'])
        self.assertEqual(d['key3'][0], d2['key3'][0])
        self.assertEqual(d['key3'][1], d2['key3'][1])

    def tearDown(self):
        self._server.shutdown()
        self._thr.join()
        self._server.server_close()
