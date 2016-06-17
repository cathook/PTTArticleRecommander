import inspect
import logging
import socket
import sys
import threading
import time
import unittest

from modules.protocol import net
from modules.protocol import types
from modules import proxy_client


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class _GoodError(Exception):
    pass

class _FakeServer(object):
    def __init__(self):
        self.normal = True
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.port = 8397
        while True:
            try:
                self._sock.bind(('localhost', self.port))
                break
            except Exception as e:
                self.port += 1
        self._sock.listen(1)
        self._stop_flag = False
        self._conn = None
        self.num_queries = 0

    def main_loop(self):
        try:
            (fd, addr) = self._sock.accept()
            self._conn = fd
            self._conn.settimeout(1)
            while True:
                ph = net.PackageHeader.load(
                        self._recv_all(net.PackageHeader.SIZE))[0]
                ph.typee ^= net.PackageType.REPLY_QUERY_BIT
                buf = self._recv_all(ph.size)
                self._conn.sendall(ph.dump() + buf)
        except _GoodError as _:
            pass
        except Exception as e:
            self.normal = False
        finally:
            try:
                self._conn.close()
            except Exception as _:
                pass
            self._sock.close()

    def stop(self):
        self._stop_flag = True

    def _recv_all(self, sz):
        ret = b''
        while len(ret) < sz and not self._stop_flag:
            try:
                a = self._conn.recv(sz - len(ret))
            except socket.timeout:
                continue
            if not a:
                raise Exception()
            ret += a
        if self._stop_flag:
            raise _GoodError()
        return ret


class TestProxyClient(unittest.TestCase):
    def runTest(self):
        server = _FakeServer()
        thr = threading.Thread(target=server.main_loop)
        thr.start()
        time.sleep(1)

        good = True
        try:
            proxy = proxy_client.ProxyClient(
                    'localhost', server.port, logging.getLogger())

            for i in range(5):
                proxy.send_pkg(1234, b'aabbccex')

                ph = proxy.recv_header()
                self.assertEqual(ph.typee,
                                 1234 ^ net.PackageType.REPLY_QUERY_BIT)

                buf = proxy.recv_all(ph.size)
            self.assertEqual(buf, b'aabbccex')
        except Exception as e:
            print('Err: %r' % e)
            good = False
        self.assertTrue(good)

        server.stop()
        thr.join()

        self.assertTrue(server.normal)
