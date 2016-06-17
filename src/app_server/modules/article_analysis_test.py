import inspect
import logging
import socket
import sys
import threading
import time
import unittest

from modules import article_analysis
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
                if ph.typee != net.PackageType.QUERY_DOC_REL_INFO:
                    print('unknown type')
                    raise Exception()
                doc_id, offs = types.DocIdentity.load(self._recv_all(ph.size))
                if offs != ph.size:
                    print('wrong length')
                    raise Exception()
                self._send_type_pkg(net.PackageType.REPLY_DOC_REL_INFO,
                                    self.get_doc_rel_info(doc_id).dump())
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

    @staticmethod
    def get_doc_rel_info(doc_id):
        return types.DocRelInfo([doc_id],
                                [doc_id, doc_id],
                                [doc_id, doc_id, doc_id])

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


class _TestArticleAnalysisCommon(unittest.TestCase):
    def _runTest(self, Creater, arr, ct):
        server = _FakeServer()
        thr = threading.Thread(target=server.main_loop)
        thr.start()
        time.sleep(1)

        aa = Creater(server.port)
        for i in arr:
            iden = types.DocIdentity('board', i)
            r = aa.get_doc_rel_info(iden)
            self._check_is_correct_doc_rel_info(iden, r)

        server.stop()
        thr.join()
        self.assertEqual(ct, server.num_queries)
        self.assertTrue(server.normal)

    def _check_is_correct_doc_rel_info(self, iden, ret):
        self.assertEqual(len(ret.pos_rel_docs), 1)
        self.assertEqual(len(ret.neg_rel_docs), 2)
        self.assertEqual(len(ret.neutral_rel_docs), 3)
        for a in ret.pos_rel_docs + ret.neg_rel_docs + ret.neutral_rel_docs:
            self.assertEqual(a.board, iden.board)
            self.assertEqual(a.idid, iden.idid)


class TestArticleAnalysisProxy(_TestArticleAnalysisCommon):
    def runTest(self):
        Meta = lambda port: article_analysis._ArticleAnalysisProxy(
                'localhost', port, logging.getLogger())
        self._runTest(Meta, [1, 2, 3, 4, 5], 5)


class TestArticleAnalysis(_TestArticleAnalysisCommon):
    def runTest(self):
        Meta = lambda port: article_analysis.ArticleAnalysis(
                'localhost', port, 5, logging.getLogger())
        self._runTest(Meta, [1, 2, 3, 4, 5, 6, 5, 6, 5, 6, 3, 1], 8)
