import inspect
import socket
import sys
import time
import threading
import unittest

from modules import server
from modules.backend_interface import BackendInterface
from modules.protocol import net
from modules.protocol import types


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class _FakeBackendServer(BackendInterface):
    def __init__(self):
        pass

    def get_max_id(self, board):
        return int(board)

    def get_doc_meta_data_after_id(self, board, idid):
        return [types.DocMetaData(idid, idid - 1,
                                  board + '_title_get_doc_meta_after_id',
                                  board + '_author',
                                  idid * 256,
                                  board,
                                  [idid + 1, idid + 2, idid + 3])]

    def get_doc_meta_data_after_time(self, board, post_time):
        idid = int(post_time / 256)
        return [types.DocMetaData(idid, idid - 1,
                                  board + '_title_get_doc_meta_after_time',
                                  board + '_author',
                                  post_time,
                                  board,
                                  [idid + 1, idid + 2, idid + 3])]

    def get_doc_meta_data_of_author(self, board, author):
        idid = int(board)
        return [types.DocMetaData(idid, idid - 1,
                                  board + '_title_get_doc_meta_data_of_author',
                                  author,
                                  idid * 256,
                                  board,
                                  [idid + 1, idid + 2, idid + 3])]

    def get_doc_meta_data_series(self, board, idid):
        return [types.DocMetaData(idid, idid - 1,
                                  board + '_title_doc_meta_data_series',
                                  board + '_author',
                                  idid * 256,
                                  board,
                                  [idid + 1, idid + 2, idid + 3])]

    def get_doc_real_data(self, board, idid):
        r1 = types.ReplyMessage(types.ReplyMode.GOOD, board + '_r1', 'r1r1')
        r2 = types.ReplyMessage(types.ReplyMode.WOO , board + '_r2', 'r2r2')
        r3 = types.ReplyMessage(types.ReplyMode.NORMAL , board + '_r3', 'r3r3')
        return types.DocRealData(board + board + board, [r1, r2, r3])

    def get_id_by_url(self, url):
        return types.DocIdentity(url, int(url))

    def get_url_by_id(self, board, idid):
        return board


class _TestDelegator(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(_TestDelegator, self).__init__(*args, **kwargs)
        self.backend = None
        self.delegator = None

    def setUp(self):
        self.backend = _FakeBackendServer()
        self.delegator = server._Delegator(self.backend)

    def tearDown(self):
        self.backend = None
        self.delegator = None


class TestDelegatorGetMaxId(_TestDelegator):
    def __init__(self, *args, **kwargs):
        super(TestDelegatorGetMaxId, self).__init__(*args, **kwargs)

    def runTest(self):
        qtype = net.PackageType.QUERY_MAX_ID
        qbuf = net.StringStruct.pack('123')
        (rtype, rbuf) = self.delegator.handle_package(qtype, qbuf)
        self.assertEqual(rtype, net.PackageType.REPLY_MAX_ID)
        self.assertEqual(rbuf, net.IdentityStruct.pack(123))


class TestDelegatorGetDocMetaDataAfterId(_TestDelegator):
    def __init__(self, *args, **kwargs):
        super(TestDelegatorGetDocMetaDataAfterId, self).__init__(
                *args, **kwargs)

    def runTest(self):
        qtype = net.PackageType.QUERY_DOC_META_DATA_AFTER_ID
        qbuf = net.StringStruct.pack('ba') + net.IdentityStruct.pack(123)
        (rtype, rbuf) = self.delegator.handle_package(qtype, qbuf)
        self.assertEqual(rtype, net.PackageType.REPLY_DOC_META_DATA_AFTER_ID)
        ansbuf = net.ArrayStruct.pack(
                self.backend.get_doc_meta_data_after_id('ba', 123),
                net.CustomStruct)
        self.assertEqual(rbuf, ansbuf)


class TestDelegatorGetDocMetaDataAfterTime(_TestDelegator):
    def __init__(self, *args, **kwargs):
        super(TestDelegatorGetDocMetaDataAfterTime, self).__init__(
                *args, **kwargs)

    def runTest(self):
        qtype = net.PackageType.QUERY_DOC_META_DATA_AFTER_TIME
        qbuf = net.StringStruct.pack('bsfs') + net.TimeStruct.pack(123 * 256)
        (rtype, rbuf) = self.delegator.handle_package(qtype, qbuf)
        self.assertEqual(rtype, net.PackageType.REPLY_DOC_META_DATA_AFTER_TIME)
        ansbuf = net.ArrayStruct.pack(
                self.backend.get_doc_meta_data_after_time('bsfs', 123 * 256),
                net.CustomStruct)
        self.assertEqual(rbuf, ansbuf)


class TestDelegatorGetDocMetaDataOfAuthor(_TestDelegator):
    def __init__(self, *args, **kwargs):
        super(TestDelegatorGetDocMetaDataOfAuthor, self).__init__(
                *args, **kwargs)

    def runTest(self):
        qtype = net.PackageType.QUERY_DOC_META_DATA_OF_AUTHOR
        qbuf = net.StringStruct.pack('10') + net.StringStruct.pack('meow')
        (rtype, rbuf) = self.delegator.handle_package(qtype, qbuf)
        self.assertEqual(rtype, net.PackageType.REPLY_DOC_META_DATA_OF_AUTHOR)
        ansbuf = net.ArrayStruct.pack(
                self.backend.get_doc_meta_data_of_author('10', 'meow'),
                net.CustomStruct)
        self.assertEqual(rbuf, ansbuf)


class TestDelegatorGetDocMetaDataSeries(_TestDelegator):
    def __init__(self, *args, **kwargs):
        super(TestDelegatorGetDocMetaDataSeries, self).__init__(*args, **kwargs)

    def runTest(self):
        qtype = net.PackageType.QUERY_DOC_META_DATA_SERIES
        qbuf = net.StringStruct.pack('aaab') + net.IdentityStruct.pack(6999)
        (rtype, rbuf) = self.delegator.handle_package(qtype, qbuf)
        self.assertEqual(rtype, net.PackageType.REPLY_DOC_META_DATA_SERIES)
        ansbuf = net.ArrayStruct.pack(
                self.backend.get_doc_meta_data_series('aaab', 6999),
                net.CustomStruct)
        self.assertEqual(rbuf, ansbuf)


class TestDelegatorGetDocRealData(_TestDelegator):
    def __init__(self, *args, **kwargs):
        super(TestDelegatorGetDocRealData, self).__init__(*args, **kwargs)

    def runTest(self):
        qtype = net.PackageType.QUERY_DOC_REAL_DATA
        qbuf = net.StringStruct.pack('ccdd') + net.IdentityStruct.pack(1235)
        (rtype, rbuf) = self.delegator.handle_package(qtype, qbuf)
        self.assertEqual(rtype, net.PackageType.REPLY_DOC_REAL_DATA)
        ansbuf = self.backend.get_doc_real_data('ccdd', 1235).dump()
        self.assertEqual(rbuf, ansbuf)


class TestDelegatorGetIdByUrl(_TestDelegator):
    def __init__(self, *args, **kwargs):
        super(TestDelegatorGetIdByUrl, self).__init__(*args, **kwargs)

    def runTest(self):
        qtype = net.PackageType.QUERY_ID_BY_URL
        qbuf = net.StringStruct.pack('12345')
        (rtype, rbuf) = self.delegator.handle_package(qtype, qbuf)
        self.assertEqual(rtype, net.PackageType.REPLY_ID_BY_URL)
        ansbuf = self.backend.get_id_by_url('12345').dump()
        self.assertEqual(rbuf, ansbuf)


class TestDelegatorGetUrlById(_TestDelegator):
    def __init__(self, *args, **kwargs):
        super(TestDelegatorGetUrlById, self).__init__(*args, **kwargs)

    def runTest(self):
        qtype = net.PackageType.QUERY_URL_BY_ID
        qbuf = net.StringStruct.pack('boarddd') + net.IdentityStruct.pack(1234)
        (rtype, rbuf) = self.delegator.handle_package(qtype, qbuf)
        self.assertEqual(rtype, net.PackageType.REPLY_URL_BY_ID)
        ansbuf = net.StringStruct.pack(
                self.backend.get_url_by_id('boarddd', 1234))
        self.assertEqual(rbuf, ansbuf)


class TestDelegatorProtocolError(_TestDelegator):
    def runTest(self):
        booo = False
        try:
            self.delegator.handle_package(123, b'')
        except server._ProtocolError as e:
            booo = True
        self.assertTrue(booo)


class _TestSocketConnectionHandler(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(_TestSocketConnectionHandler, self).__init__(*args, **kwargs)

    def setUp(self):
        self.sock_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        p = 8937
        while True:
            try:
                self.sock_server.bind(('localhost', p))
                break
            except Exception as e:
                p += 1
        self.sock_server.listen(10)
        self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        t = threading.Thread(target=self._accept_thread)
        t.start()
        self.conn.connect(('localhost', p))
        t.join()
        self.backend = _FakeBackendServer()

    def tearDown(self):
        self.handler = None
        self.conn.close()
        self.conn = None
        self.sock_server_conn.close()
        self.sock_server_conn = None
        self.sock_server.close()
        self.sock_server = None

    def _accept_thread(self):
        (self.sock_server_conn, _) = self.sock_server.accept()

    def do_a_test(self, t):
        self.conn.sendall(
                net.PackageHeader(net.PackageType.QUERY_MAX_ID, 10).dump())
        time.sleep(t)
        self.conn.sendall(b'\x02\x00\x00\x00\x00\x00\x00\x0013')
        (h, _) = net.PackageHeader.load(self.conn.recv(net.PackageHeader.SIZE))
        self.assertEqual(h.typee, net.PackageType.REPLY_MAX_ID)
        self.assertEqual(h.size, 4)
        (a, _) = net.IdentityStruct.unpack(self.conn.recv(4))
        self.assertEqual(a, 13)

    def do_a_bad_test(self):
        self.conn.sendall(net.PackageHeader(29958, 1).dump() +
                          b'\x02\x00\x00\x00\x00\x00\x00\x0013')

    def do_a_test_small(self, sz):
        buf = net.PackageHeader(net.PackageType.QUERY_MAX_ID, 10).dump() + \
                b'\x02\x00\x00\x00\x00\x00\x00\x0013'
        self.conn.sendall(buf[ : sz])


class TestSocketConnectionHandlerNormal(_TestSocketConnectionHandler):
    def __init__(self, *a, **kwa):
        super(TestSocketConnectionHandlerNormal, self).__init__(*a, **kwa)

    def runTest(self):
        self._stopped = False
        handler = server._SocketConnectionHandler(
               self.backend, self.sock_server_conn, self._notifier)
        handler.start()

        self.do_a_test(0)
        self.do_a_test(3)

        handler.stop()
        handler.join()
        self.assertFalse(self._stopped)

    def _notifier(self, conn):
        self._stopped = True


class TestSocketConnectionHandlerBadProtocol(_TestSocketConnectionHandler):
    def __init__(self, *a, **kwa):
        super(TestSocketConnectionHandlerBadProtocol, self).__init__(*a, **kwa)

    def runTest(self):
        self._stopped = False
        handler = server._SocketConnectionHandler(
               self.backend, self.sock_server_conn, self._notifier)
        handler.start()

        self.do_a_bad_test()

        handler.join()
        self.assertTrue(self._stopped)

    def _notifier(self, conn):
        self._stopped = True


class TestSocketConnectionHandlerSocketError(_TestSocketConnectionHandler):
    def __init__(self, *a, **kwa):
        super(TestSocketConnectionHandlerSocketError, self).__init__(*a, **kwa)

    def runTest(self):
        self._stopped = False
        handler = server._SocketConnectionHandler(
               self.backend, self.sock_server_conn, self._notifier)
        handler.start()

        self.do_a_test_small(3)
        self.conn.close()

        handler.join()
        self.assertTrue(self._stopped)

    def _notifier(self, conn):
        self._stopped = True


class TestSocketConnectionHandlerStop(_TestSocketConnectionHandler):
    def __init__(self, *a, **kwa):
        super(TestSocketConnectionHandlerStop, self).__init__(*a, **kwa)

    def runTest(self):
        for sz in range(30):
            self._stopped = False
            handler = server._SocketConnectionHandler(
                    self.backend, self.sock_server_conn, self._notifier,
                    auto_close=False)
            handler.start()
            self.do_a_test_small(sz)
            handler.stop()
            handler.join()
            self.assertFalse(self._stopped)
            try:
                self.sock_server_conn.settimeout(0.2)
                self.sock_server_conn.recv(10000)
            except socket.timeout as e:
                pass

    def _notifier(self, conn):
        self._stopped = True


class TestServer(unittest.TestCase):
    def __init__(self, *a, **kwa):
        super(TestServer, self).__init__(*a, **kwa)

    def runTest(self):
        self.ps = server.Server(_FakeBackendServer(), 'localhost', 8394)
        thr = threading.Thread(target=self._run_thread)
        thr.start()
        c1 = self._get_conn()
        c2 = self._get_conn()
        self._do_a_test(c1)
        c3 = self._get_conn()
        self._do_a_bad_test(c2)
        self._do_a_test(c3)
        c4 = self._get_conn()
        self._do_a_bad_test(c4)
        time.sleep(2)
        self.ps.stop()
        thr.join()
        self.assertTrue(True)

    def _get_conn(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('localhost', 8394))
        return s

    def _run_thread(self):
        self.ps.run()

    def _do_a_test(self, c):
        c.sendall(net.PackageHeader(net.PackageType.QUERY_MAX_ID, 10).dump())
        c.sendall(b'\x02\x00\x00\x00\x00\x00\x00\x0013')
        (h, _) = net.PackageHeader.load(c.recv(net.PackageHeader.SIZE))
        self.assertEqual(h.typee, net.PackageType.REPLY_MAX_ID)
        self.assertEqual(h.size, 4)
        (a, _) = net.IdentityStruct.unpack(c.recv(4))
        self.assertEqual(a, 13)

    def _do_a_bad_test(self, c):
        c.sendall(net.PackageHeader(29958, 1).dump() +
                  b'\x02\x00\x00\x00\x00\x00\x00\x0013')
