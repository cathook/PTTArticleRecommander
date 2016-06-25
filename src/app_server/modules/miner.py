import logging
import socket
import threading

from modules.protocol import net
from modules.protocol import types
from modules import proxy_client


class MinerError(Exception):
    def __init__(self, *args, **kwargs):
        super(MinerError, self).__init__(*args, **kwargs)


class _MinerProxy(object):
    '''A proxy to the real miner server.

    Attributes:
        _logger: The logger.
        _lock: A lock to prevent race condition on the internet communicate.
        _proxy_client: An instance of `proxy_client.ProxyClient`
    '''
    def __init__(self, addr, port, logger):
        '''Constructor.

        Args:
            addr: Address of the server.
            port: Port of the server.
            logger: The logger.
        '''
        self._logger = logger
        self._lock = threading.Lock()
        try:
            self._proxy_client = proxy_client.ProxyClient(addr, port,
                                                          self._logger)
        except proxy_client.ProxyClientError as e:
            self._logger.error('Cannot connect to the miner server.')
            raise

    def get_url_by_doc_identity(self, identity):
        '''Gets a documnent url by its identity.

        Args:
            identity: The document's identity.
        Returns: The document's url.
        '''
        with self._lock:
            self._logger.info('Query url by doc identity.')
            self._proxy_client.send_pkg(
                    net.PackageType.QUERY_URL_BY_ID, identity.dump())
            return self._recv_and_unpack(net.PackageType.REPLY_URL_BY_ID,
                                         net.StringStruct.unpack)

    def get_doc_identity_by_url(self, url):
        '''Gets a documnent identity by its url.

        Args:
            url: The document's url.
        Returns: The document's identity.
        '''
        with self._lock:
            self._logger.info('Query doc identity by url.')
            self._proxy_client.send_pkg(net.PackageType.QUERY_ID_BY_URL,
                                        net.StringStruct.pack(url))
            return self._recv_and_unpack(net.PackageType.REPLY_ID_BY_URL,
                                         types.DocIdentity.load)

    def _recv_and_unpack(self, typee, unpack_func):
        ph = self._proxy_client.recv_header()
        if ph.typee != typee:
            raise MinerError('Wrong return package type.')
        buf = self._proxy_client.recv_all(ph.size)
        (out, offs) = unpack_func(buf)
        if offs != ph.size:
            raise Miner('Wrong format.')
        return out


class Miner(object):
    '''A miner which knows the mapping between document id and its url.

    Attributes:
        _miner_proxy: The real miner server.
        _max_cached_size: The maximum number of cached data.
        _cache: The cached mapping.
        _logger: The logger.
    '''
    def __init__(self, addr, port, max_cached_size, logger):
        '''Constructor.

        Args:
            addr: Address of the real server.
            port: Port of the real server.
            max_cached_size: Maximum cached size.
            logger: The logger.
        '''
        self._logger = logger
        self._max_cached_size = max_cached_size
        self._cache = []
        self._miner_proxy = _MinerProxy(addr, port,
                                        self._logger.getChild('proxy'))

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
        self._add_to_cache(url, identity)
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
        self._add_to_cache(url, identity)
        self._ensure_cached_size_acceptable()
        return identity

    def _add_to_cache(self, url, identity):
        self._logger.info('Add %s:%d <=> %s to cache'
                          % (identity.board, identity.idid, url))
        self._cache.append(
                (url, types.DocIdentity(identity.board, identity.idid)))

    def _ensure_cached_size_acceptable(self):
        if len(self._cache) > self._max_cached_size:
            self._logger.info('Cache fulled, shrink.')
            del self._cache[ : int((self._max_cached_size + 1) / 2)]
