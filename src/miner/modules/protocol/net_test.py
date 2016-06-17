import inspect
import sys
import unittest

from modules.protocol import net


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    for (name, obj) in inspect.getmembers(sys.modules[__name__]):
        if name.startswith('Test'):
            tests = loader.loadTestsFromTestCase(obj)
            suite.addTests(tests)
    return suite


class TestPackageHeader(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestPackageHeader, self).__init__(*args, **kwargs)

    def runTest(self):
        header = net.PackageHeader(7, 13)
        buf = b'\x07\x00\x00\x00' + b'\x0d\x00\x00\x00\x00\x00\x00\x00'

        self.assertEqual(header.SIZE, len(buf))
        self.assertEqual(header.dump(), buf)

        header2, offs = net.PackageHeader.load(buf)
        self.assertEqual(offs, len(buf))
        self.assertEqual(header2.typee, header.typee)
        self.assertEqual(header2.size, header.size)


class TestStringStruct(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestStringStruct, self).__init__(*args, **kwargs)

    def runTest(self):
        s = 'meow'
        buf = b'\x04\x00\x00\x00\x00\x00\x00\x00meow'
        self.assertEqual(net.StringStruct.pack(s), buf)
        self.assertEqual(net.StringStruct.unpack(b' ' + buf, 1), (s, 8 + 4 + 1))

        s = 'meow中文'
        buf = b'meow\xe4\xb8\xad\xe6\x96\x87'
        l = len(buf)
        buff = bytes([l]) + b'\x00' * 7 + buf
        self.assertEqual(net.StringStruct.pack(s), buff)
        self.assertEqual(net.StringStruct.unpack(buff, 0), (s, 8 + l))


class TestArrayStruct(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestArrayStruct, self).__init__(*args, **kwargs)

    def runTest(self):
        arr = ['a', 'b', 'c']
        buf = b'\x03\x00\x00\x00\x00\x00\x00\x00abc'
        self.assertEqual(net.ArrayStruct.pack(arr, self._S()), buf)
        arr2 = net.ArrayStruct.unpack(buf, 0, self._S())[0]
        self.assertEqual(len(arr), len(arr2))
        for i in range(len(arr)):
            self.assertEqual(arr[i], arr2[i])

    class _S(object):
        def pack(self, c):
            return bytes([ord(c)])

        def unpack(self, buf, offs=0):
            return (chr(buf[offs]), offs + 1)


class TestIdentityStruct(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestIdentityStruct, self).__init__(*args, **kwargs)

    def runTest(self):
        idid = 134
        buf = b'\x86\x00\x00\x00'
        self.assertEqual(net.IdentityStruct.pack(idid), buf)

        self.assertEqual(net.IdentityStruct.unpack(b'   ' + buf, 3), (idid, 7))
        self.assertEqual(net.IdentityStruct.unpack(buf), (idid, 4))


class TestTimeStruct(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestTimeStruct, self).__init__(*args, **kwargs)

    def runTest(self):
        t = 134
        buf = b'\x86\x00\x00\x00\x00\x00\x00\x00'
        self.assertEqual(net.TimeStruct.pack(t), buf)

        self.assertEqual(net.TimeStruct.unpack(b'   ' + buf, 3), (t, 8 + 3))
        self.assertEqual(net.TimeStruct.unpack(buf), (t, 8))


class TestCustomStruct(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestCustomStruct, self).__init__(*args, **kwargs)

    def runTest(self):
        header = net.PackageHeader(7, 13)
        buf = b'\x07\x00\x00\x00' + b'\x0d\x00\x00\x00\x00\x00\x00\x00'

        self.assertEqual(header.SIZE, len(buf))
        self.assertEqual(net.CustomStruct.pack(header), buf)

        h2, offs = net.CustomStruct(net.PackageHeader).unpack(buf)
        self.assertEqual(offs, len(buf))
        self.assertEqual(h2.typee, header.typee)
        self.assertEqual(h2.size, header.size)
