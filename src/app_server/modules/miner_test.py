import inspect
import logging
import socket
import sys
import threading
import time
import unittest

from modules import miner
from modules.protocol import net
from modules.protocol import types


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
                if ph.typee == net.PackageType.QUERY_URL_BY_ID:
                    self._handle_query_url_by_id(ph)
                elif ph.typee == net.PackageType.QUERY_ID_BY_URL:
                    self._handle_query_id_by_url(ph)
                else:
                    print('unknown type')
                    raise Exception()
                self.num_queries += 1
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

    def _handle_query_url_by_id(self, ph):
        doc_id, offs = types.DocIdentity.load(self._recv_all(ph.size))
        if offs != ph.size:
            raise Exception()
        url = doc_id.board + '%d' % doc_id.idid
        self._send_type_pkg(
                net.PackageType.REPLY_URL_BY_ID, net.StringStruct.pack(url))

    def _handle_query_id_by_url(self, ph):
        url, offs = net.StringStruct.unpack(self._recv_all(ph.size))
        if offs != ph.size:
            raise Exception()
        doc_id = types.DocIdentity(url, int(url))
        self._send_type_pkg(net.PackageType.REPLY_ID_BY_URL, doc_id.dump())

    def _send_type_pkg(self, typee, buf):
        ph = net.PackageHeader(typee, len(buf))
        self._conn.sendall(ph.dump() + buf)

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


class TestMinerProxy(unittest.TestCase):
    def runTest(self):
        server = _FakeServer()
        thr = threading.Thread(target=server.main_loop)
        thr.start()
        time.sleep(1)

        miner_proxy = miner._MinerProxy('localhost', server.port,
                                        logging.getLogger())
        u = miner_proxy.get_url_by_doc_identity(types.DocIdentity('board',
                                                                  1234))
        self.assertEqual(u, 'board1234')

        di = miner_proxy.get_doc_identity_by_url('12345')
        self.assertEqual(di.board, '12345')
        self.assertEqual(di.idid, 12345)

        server.stop()
        thr.join()

        self.assertTrue(server.normal)


class TestMiner(unittest.TestCase):
    def setUp(self):
        self._server = _FakeServer()
        self._thr = threading.Thread(target=self._server.main_loop)
        self._thr.start()
        time.sleep(1)

    def runTest(self):
        m = miner.Miner('localhost', self._server.port, 3, logging.getLogger())
        ct = 0

        u = m.get_url_by_doc_identity(types.DocIdentity('board', 11244))
        self.assertEqual(u, 'board11244')
        ct += 1

        for i in range(20):
            u = m.get_url_by_doc_identity(types.DocIdentity('board', 11244))
            self.assertEqual(u, 'board%d' % 11244)

        di = m.get_doc_identity_by_url('25377')
        self.assertTrue(di.board == '25377' and di.idid == 25377)
        ct += 1

        di = m.get_doc_identity_by_url('25357')
        self.assertTrue(di.board == '25357' and di.idid == 25357)
        ct += 1

        di = m.get_doc_identity_by_url('25353')
        self.assertTrue(di.board == '25353' and di.idid == 25353)
        ct += 1

        u = m.get_url_by_doc_identity(types.DocIdentity('board', 11244))
        self.assertEqual(u, 'board11244')
        ct += 1

        di = m.get_doc_identity_by_url('25353')
        self.assertTrue(di.board == '25353' and di.idid == 25353)

        self.assertEqual(ct, self._server.num_queries)

    def tearDown(self):
        self._server.stop()
        self._thr.join()
