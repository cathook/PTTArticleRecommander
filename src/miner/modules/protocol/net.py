import struct


# Enumerates of the type of the package.
class PackageType(object):
    ASYNC_BIT = 0x100000
    REPLY_QUERY_BIT = 0x01

    QUERY_MAX_ID = 0x100
    REPLY_MAX_ID = QUERY_MAX_ID | REPLY_QUERY_BIT

    QUERY_DOC_META_DATA_AFTER_ID = 0x200
    REPLY_DOC_META_DATA_AFTER_ID = \
            QUERY_DOC_META_DATA_AFTER_ID | REPLY_QUERY_BIT

    QUERY_DOC_META_DATA_AFTER_TIME = 0x300
    REPLY_DOC_META_DATA_AFTER_TIME = \
            QUERY_DOC_META_DATA_AFTER_TIME | REPLY_QUERY_BIT

    QUERY_DOC_META_DATA_OF_AUTHOR = 0x400
    REPLY_DOC_META_DATA_OF_AUTHOR = \
            QUERY_DOC_META_DATA_OF_AUTHOR  | REPLY_QUERY_BIT

    QUERY_DOC_META_DATA_SERIES = 0x500
    REPLY_DOC_META_DATA_SERIES = QUERY_DOC_META_DATA_SERIES | REPLY_QUERY_BIT

    QUERY_DOC_REAL_DATA = 0x600
    REPLY_DOC_REAL_DATA = QUERY_DOC_REAL_DATA | REPLY_QUERY_BIT

    REGISTER_NEW_DOC_LISTENER = 0x700 | ASYNC_BIT
    UNREGISTER_NEW_DOC_LISTENER = 0x710 | ASYNC_BIT
    NOTIFY_NEW_DOC_LISTENER = 0x720

    REGISTER_DOC_META_DATA_CHANGED_LISTENER = 0x800 | ASYNC_BIT
    UNREGISTER_DOC_META_DATA_CHANGED_LISTENER = 0x810 | ASYNC_BIT
    NOTIFY_DOC_META_DATA_CHANGED_LISTENER = 0x820

    QUERY_URL_BY_ID = 0x900
    REPLY_URL_BY_ID = QUERY_URL_BY_ID | REPLY_QUERY_BIT

    QUERY_ID_BY_URL = 0xa00
    REPLY_ID_BY_URL = QUERY_ID_BY_URL | REPLY_QUERY_BIT

    QUERY_DOC_REL_INFO = 0xb00
    REPLY_DOC_REL_INFO = QUERY_DOC_REL_INFO | REPLY_QUERY_BIT


class PackageHeader(object):
    '''A struct of package's header'''
    SIZE = 4 + 8
    _STRUCT = struct.Struct('<LQ')
    def __init__(self, typee, size):
        self.typee = typee
        self.size = size

    def dump(self):
        return self._STRUCT.pack(self.typee, self.size)

    @staticmethod
    def load(buf, offs=0):
        (typee, size) = PackageHeader._STRUCT.unpack(
                 buf[offs : offs + PackageHeader.SIZE])
        a = PackageHeader(typee, size)
        offs += PackageHeader.SIZE
        return (a, offs)


class StringStruct(object):
    '''Struct for string'''
    @staticmethod
    def pack(s):
        b = s.encode('utf-8')
        return struct.pack('<Q%ds' % len(b), len(b), b)

    @staticmethod
    def unpack(buf, offs=0):
        l = struct.unpack('<Q', buf[offs : offs + 8])[0]
        offs += 8
        s = struct.unpack('<%ds' % l, buf[offs : offs + l])[0].decode('utf-8')
        offs += l
        return (s, offs)


class ArrayStruct(object):
    '''Struct for array'''
    @staticmethod
    def pack(a, element_struct):
        x = struct.pack('<Q', len(a))
        y = b''.join(element_struct.pack(b) for b in a)
        return x + y

    @staticmethod
    def unpack(buf, offs, element_struct):
        x = struct.unpack('<Q', buf[offs : offs + 8])[0]
        offs += 8
        ret = []
        for i in range(x):
            (a, offs) = element_struct.unpack(buf, offs)
            ret.append(a)
        return (ret, offs)


class IdentityStruct(object):
    '''Struct for document idnetity'''
    @staticmethod
    def pack(idid):
        return struct.pack('<L', idid)

    @staticmethod
    def unpack(buf, offs=0):
        return (struct.unpack('<L', buf[offs : offs + 4])[0], offs + 4)


class TimeStruct(object):
    '''Struct for time'''
    @staticmethod
    def pack(t):
        return struct.pack('<Q', t)

    @staticmethod
    def unpack(buf, offs=0):
        return (struct.unpack('<Q', buf[offs : offs + 8])[0], offs + 8)


class ReplyModeStruct(object):
    '''Struct for reply mode number'''
    @staticmethod
    def pack(a):
        return struct.pack('<H', a)


class CustomStruct(object):
    '''A struct which calls the input structure's dump method for packing.'''
    def __init__(self, meta):
        self._meta = meta

    @staticmethod
    def pack(a):
        return a.dump()

    def unpack(self, buf, offs=0):
        return self._meta.load(buf, offs)
