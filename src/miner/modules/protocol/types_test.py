import inspect
import sys
import unittest

from modules.protocol import types


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class TestReplyMessage(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestReplyMessage, self).__init__(*args, **kwargs)

    def runTest(self):
        rm = types.ReplyMessage(types.ReplyMode.NORMAL, 'meow', 'wang wang')
        buf = b''.join([b'\x01\x00',
                        b'\x04\x00\x00\x00\x00\x00\x00\x00meow',
                        b'\x09\x00\x00\x00\x00\x00\x00\x00wang wang'])
        self.assertEqual(rm.dump(), buf)


class TestDocMetaData(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestDocMetaData, self).__init__(*args, **kwargs)

    def runTest(self):
        dmd = types.DocMetaData(5, 3, 'meow', 'wang', 9, 'blah', [2, 5, 7])
        buf = b''.join([b'\x05\x00\x00\x00',
                        b'\x03\x00\x00\x00',
                        b'\x04\x00\x00\x00\x00\x00\x00\x00meow',
                        b'\x04\x00\x00\x00\x00\x00\x00\x00wang',
                        b'\x09\x00\x00\x00\x00\x00\x00\x00',
                        b'\x04\x00\x00\x00\x00\x00\x00\x00blah',
                        b'\x02\x00\x00\x00',
                        b'\x05\x00\x00\x00',
                        b'\x07\x00\x00\x00'])
        print('')
        print(dmd.dump())
        print(buf)
        self.assertEqual(dmd.dump(), buf)


class TestDocIdentity(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestDocIdentity, self).__init__(*args, **kwargs)

    def runTest(self):
        di = types.DocIdentity('blabla~', 18)
        buf = b''.join([b'\x07\x00\x00\x00\x00\x00\x00\x00blabla~',
                        b'\x12\x00\x00\x00'])
        self.assertEqual(di.dump(), buf)

        (di, offs) = types.DocIdentity.load(buf)
        self.assertEqual(offs, 8 + 7 + 4)
        self.assertEqual(di.board, 'blabla~')
        self.assertEqual(di.idid, 18)


class TestDocRealData(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestDocRealData, self).__init__(*args, **kwargs)

    def runTest(self):
        r1 = types.ReplyMessage(2, 'a', 'bb')
        r2 = types.ReplyMessage(1, 'b', 'cc')
        drd = types.DocRealData('wu lala', [r1, r2])
        buf = b''.join([b'\x07\x00\x00\x00\x00\x00\x00\x00wu lala',
                        b'\x02\x00\x00\x00\x00\x00\x00\x00',
                        r1.dump(), r2.dump()])
        self.assertEqual(drd.dump(), buf)


class TestDocRelInfo(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestDocRelInfo, self).__init__(*args, **kwargs)

    def runTest(self):
        dri = types.DocRelInfo([], [], [types.DocIdentity('board', 123)])
        buf = b'\x00\x00\x00\x00\x00\x00\x00\x00' + \
                b'\x00\x00\x00\x00\x00\x00\x00\x00' + \
                b'\x01\x00\x00\x00\x00\x00\x00\x00' + \
                types.DocIdentity('board', 123).dump()

        buf2 = dri.dump()
        self.assertEqual(buf, buf2)

        dri2 = types.DocRelInfo.load(buf)[0]
        self.assertEqual(len(dri2.pos_rel_docs), 0)
        self.assertEqual(len(dri2.neg_rel_docs), 0)
        self.assertEqual(len(dri2.neutral_rel_docs), 1)
        self.assertEqual(dri2.neutral_rel_docs[0].board, 'board')
        self.assertEqual(dri2.neutral_rel_docs[0].idid, 123)
