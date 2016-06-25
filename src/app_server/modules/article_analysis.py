import logging
import socket
import threading

from modules.protocol import net
from modules.protocol import types
from modules import proxy_client


class ArticleAnalysisError(Exception):
    def __init__(self, *args, **kwargs):
        super(ArticleAnalysisError, self).__init__(*args, **kwargs)


class _ArticleAnalysisProxy(object):
    '''A proxy to the real article analysis server.

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

    def get_doc_rel_info(self, identity):
        '''Gets a documnent's relationship information by its identity.

        Args:
            identity: The document's identity.
        Returns: The document rel info.
        '''
        with self._lock:
            self._logger.info('Query get doc rel info.')
            self._proxy_client.send_pkg(
                    net.PackageType.QUERY_DOC_REL_INFO, identity.dump())
            return self._recv_and_unpack(net.PackageType.REPLY_DOC_REL_INFO,
                                         types.DocRelInfo.load)

    def _recv_and_unpack(self, typee, unpack_func):
        ph = self._proxy_client.recv_header()
        if ph.typee != typee:
            raise ArticleAnalysisError('Wrong return package type.')
        buf = self._proxy_client.recv_all(ph.size)
        (out, offs) = unpack_func(buf)
        if offs != ph.size:
            raise ArticleAnalysisError('Wrong format.')
        return out


class ArticleAnalysis(object):
    '''An article analysis which knows document relationship.

    Attributes:
        _proxy: The real article analysis server.
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
        self._proxy = _ArticleAnalysisProxy(addr, port,
                                            self._logger.getChild('proxy'))

    def get_doc_rel_info(self, identity):
        '''Gets a documnent's relationship information by its identity.

        Args:
            identity: The document's identity.
        Returns: The document rel info.
        '''
        for (c_identity, c_doc_rel_info) in self._cache:
            if identity.board == c_identity.board and \
                    identity.idid == c_identity.idid:
                return c_doc_rel_info
        doc_rel_info = self._proxy.get_doc_rel_info(identity)
        self._cache.append((identity,
                            types.DocRelInfo(doc_rel_info.pos_rel_docs,
                                             doc_rel_info.neg_rel_docs,
                                             doc_rel_info.neutral_rel_docs)))
        self._ensure_cached_size_acceptable()
        return doc_rel_info

    def _ensure_cached_size_acceptable(self):
        if len(self._cache) > self._max_cached_size:
            del self._cache[ : int((self._max_cached_size + 1) / 2)]
