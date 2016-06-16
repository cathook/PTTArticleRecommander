import logging
import socket
import threading

from modules.protocol import net
from modules.protocol import types


class MinerError(Exception):
    def __init__(self, *args, **kwargs):
        super(MinerError, self).__init__(*args, **kwargs)


class _MinerProxy(object):
    '''A proxy to the real miner server.

    Attributes:
        _sock: The socket object.
        _lock: A lock to prevent race condition on the internet communicate.
    '''
    def __init__(self, addr, port):
        '''Constructor.

        Args:
            addr: Address of the server.
            port: Port of the server.
        '''
        self._sock = None
        self._lock = threading.Lock()

        self._init_sock(addr, port)

    def get_url_by_doc_identity(self, identity):
        '''Gets a documnent url by its identity.

        Args:
            identity: The document's identity.
        Returns: The document's url.
        '''
        with self._lock:
            try:
                self._send_pkg(net.PackageType.QUERY_URL_BY_ID, identity.dump())
                ph = self._recv_header()
                if ph.typee != net.PackageType.REPLY_URL_BY_ID:
                    raise MinerError('Wrong protocol.')
                return self._recv_and_unpack(ph.size, net.StringStruct.unpack)
            except socket.error as e:
                raise MinerError('Cannot send: %r' % e)

    def get_doc_identity_by_url(self, url):
        '''Gets a documnent identity by its url.

        Args:
            url: The document's url.
        Returns: The document's identity.
        '''
        with self._lock:
            try:
                self._send_pkg(net.PackageType.QUERY_ID_BY_URL,
                               net.StringStruct.pack(url))
                ph = self._recv_header()
                if ph.typee != net.PackageType.REPLY_ID_BY_URL:
                    raise MinerError('Wrong protocol.')
                return self._recv_and_unpack(ph.size, types.DocIdentity.load)
            except socket.error as e:
                raise MinerError('Cannot send: %r' % e)

    def _init_sock(self, addr, port):
        try:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._sock.connect((addr, port))
        except socket.error as e:
            raise MinerError('Cannot connect to the miner server %r' % e)

    def _send_pkg(self, typee, buf):
        try:
            self._sock.sendall(net.PackageHeader(typee, len(buf)).dump() + buf)
        except socket.error as e:
            raise MinerError('Cannot send message.')

    def _recv_header(self):
        try:
            buf = self._recv_all(net.PackageHeader.SIZE)
        except MinerError as e:
            raise MinerError('Receiving PackageHeader error: %r' % e)
        return net.PackageHeader.load(buf)[0]
    
    def _recv_and_unpack(self, sz, unpack_func):
        try:
            (ret, offs) = unpack_func(self._recv_all(sz))
        except MinerError as e:
            raise MinerError('Cannot recv package: %r' % e)
        if offs != sz:
            raise MinerError('Wrong format')
        return ret

    def _recv_all(self, sz):
        buf = b''
        while len(buf) < sz:
            try:
                a = self._sock.recv(sz - len(buf))
            except socket.error as e:
                raise MinerError('Cannot recv: %r' % e)
            if not a:
                raise MinerError('Cannot recv.')
            buf += a
        return buf


class Miner(object):
    '''A miner which knows the mapping between document id and its url.

    Attributes:
        _miner_proxy: The real miner server.
        _max_cached_size: The maximum number of cached data.
        _cache: The cached mapping.
    '''
    def __init__(self, addr, port, max_cached_size):
        '''Constructor.

        Args:
            addr: Address of the real server.
            port: Port of the real server.
            max_cached_size: Maximum cached size.
        '''
        self._miner_proxy = _MinerProxy(addr, port)
        self._max_cached_size = max_cached_size
        self._cache = []

    def get_url_by_doc_identity(self, identity):
        '''Gets a documnent url by its identity.

        Args:
            identity: The document's identity.
        Returns: The document's url.
        '''
        for (c_url, c_identity) in self._cache:
            if identity.board == c_identity.board and \
                    identity.idid == c_identity.idid:
                return c_url
        url = self._miner_proxy.get_url_by_doc_identity(identity)
        self._cache.append((url,
                            types.DocIdentity(identity.board, identity.idid)))
        self._ensure_cached_size_acceptable()
        return url

    def get_doc_identity_by_url(self, url):
        '''Gets a documnent identity by its url.

        Args:
            url: The document's url.
        Returns: The document's identity.
        '''
        for (c_url, c_identity) in self._cache:
            if url == c_url:
                return types.DocIdentity(c_identity.board, c_identity.idid)
        identity = self._miner_proxy.get_doc_identity_by_url(url)
        self._cache.append((url,
                            types.DocIdentity(identity.board, identity.idid)))
        self._ensure_cached_size_acceptable()
        return identity

    def _ensure_cached_size_acceptable(self):
        if len(self._cache) > self._max_cached_size:
            del self._cache[ : int((self._max_cached_size + 1) / 2)]
