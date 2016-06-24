import logging
import socket
import threading
import time

from modules.protocol import net
from modules.protocol import types
from modules.protocol.net import PackageType
from modules import utils


class _ProtocolError(Exception):
    def __init__(self, *args, **kwargs):
        super(_ProtocolError, self).__init__(*args, **kwargs)


class _Delegator(object):
    '''A delegator which delegates the query to the backend server.

    It will transform the byte-array form query into arguments and call a
    specific method of the backend server.  And then transform the return
    value back to byte-array form.

    Static attributes:
        _handlers: A map stores each package type and it corrosponding
                method in the backend server.

    Attributes:
        backend: Reference of the backend server.
    '''
    _handlers = {}

    def __init__(self, backend):
        '''Constructor.

        Args:
            backend: Reference of the backend server.
        '''
        self._backend = backend

    def handle_package(self, typee, args_buf):
        '''Handles a specific type package.

        Args:
            typee: Type of the package content.
            args_buf: Byte-array form arguments.

        Returns: A (type, package content) touple for the reply message.
                `None` if there is no-reply message.
        '''
        if typee not in self._handlers:
            raise _ProtocolError('Unsupported query type')
        ret_type = typee | PackageType.REPLY_QUERY_BIT
        return ret_type, self._handlers[typee](self, args_buf)

    @staticmethod
    def _unpack_args_buf(args_buf, *structs):
        '''Unpacks the byte-array form arguments into argument list.

        Args:
            args_buf: byte-array arguments.
            structs: A list of struct to unpack the argument.

        Returns: A list of arguments.
        '''
        offs = 0
        args = []
        for struct in structs:
            try:
                arg, offs = struct.unpack(args_buf, offs)
            except:
                raise _ProtocolError('Cannot unpack the arguments')
            args.append(arg)
        if offs != len(args_buf):
            raise _ProtocolError('Cannot unpack the arguments')
        return args

    def _handle_query_max_id(self, args_buf):
        args = self._unpack_args_buf(args_buf, net.StringStruct)
        idid = self._backend.get_max_id(*args)
        return net.IdentityStruct.pack(idid)
    _handlers[PackageType.QUERY_MAX_ID] = _handle_query_max_id

    def _handle_query_doc_meta_data_after_id(self, args_buf):
        args = self._unpack_args_buf(args_buf,
                                     net.StringStruct, net.IdentityStruct)
        doc_meta_datas = self._backend.get_doc_meta_data_after_id(*args)
        return net.ArrayStruct.pack(doc_meta_datas, net.CustomStruct)
    _handlers[PackageType.QUERY_DOC_META_DATA_AFTER_ID] = \
            _handle_query_doc_meta_data_after_id

    def _handle_query_doc_meta_data_after_time(self, args_buf):
        args = self._unpack_args_buf(args_buf,
                                     net.StringStruct, net.TimeStruct)
        doc_meta_datas = self._backend.get_doc_meta_data_after_time(*args)
        return net.ArrayStruct.pack(doc_meta_datas, net.CustomStruct)
    _handlers[PackageType.QUERY_DOC_META_DATA_AFTER_TIME] = \
            _handle_query_doc_meta_data_after_time

    def _handle_query_doc_meta_data_of_author(self, args_buf):
        args = self._unpack_args_buf(args_buf,
                                     net.StringStruct, net.StringStruct)
        doc_meta_datas = self._backend.get_doc_meta_data_of_author(*args)
        return net.ArrayStruct.pack(doc_meta_datas, net.CustomStruct)
    _handlers[PackageType.QUERY_DOC_META_DATA_OF_AUTHOR] = \
            _handle_query_doc_meta_data_of_author

    def _handle_query_doc_meta_data_series(self, args_buf):
        args = self._unpack_args_buf(args_buf,
                                     net.StringStruct, net.IdentityStruct)
        doc_meta_datas = self._backend.get_doc_meta_data_series(*args)
        return net.ArrayStruct.pack(doc_meta_datas, net.CustomStruct)
    _handlers[PackageType.QUERY_DOC_META_DATA_SERIES] = \
            _handle_query_doc_meta_data_series

    def _handle_query_doc_real_data(self, args_buf):
        args = self._unpack_args_buf(args_buf,
                                     net.StringStruct, net.IdentityStruct)
        doc_real_data = self._backend.get_doc_real_data(*args)
        return net.CustomStruct.pack(doc_real_data)
    _handlers[PackageType.QUERY_DOC_REAL_DATA] = _handle_query_doc_real_data

    # TODO(cathook): Implement below 4 functions and the corrospinding
    #         notification handlers.
    def _handle_query_register_new_doc_listener(self, args_buf):
        raise _ProtocolError()

    def _handle_query_unregister_new_doc_listener(self, args_buf):
        raise _ProtocolError()

    def _handle_query_register_doc_meta_data_changed_listener(self, args_buf):
        raise _ProtocolError()

    def _handle_query_unregister_doc_meta_data_changed_listener(self, args_buf):
        raise _ProtocolError()

    def _handle_query_id_by_url(self, args_buf):
        args = self._unpack_args_buf(args_buf, net.StringStruct)
        doc_id = self._backend.get_id_by_url(*args)
        return net.CustomStruct.pack(doc_id)
    _handlers[PackageType.QUERY_ID_BY_URL] = _handle_query_id_by_url

    def _handle_query_url_by_id(self, args_buf):
        args = self._unpack_args_buf(args_buf,
                                     net.StringStruct, net.IdentityStruct)
        url = self._backend.get_url_by_id(*args)
        return net.StringStruct.pack(url)
    _handlers[PackageType.QUERY_URL_BY_ID] = _handle_query_url_by_id


class _SocketConnectionHandler(threading.Thread):
    '''
    A thread which handling a connection.

    Attributes:
        _delegator: The delegator which will delegate the package to the
                backend server and return the return message.
        _connection: The socket connection to handle.
        _stop_flag: A flag for whether the thread should stop or not.
        _end_notify_func: The callback function which will be called when this
                thread is end.
        _logger: A logger.
        _auto_close: A flag means whether this class should close the socket
                at the end or not.
    '''
    def __init__(self, backend, connection, end_notify_func, auto_close=True):
        '''Constructor.

        Args:
            backend: Reference of the backend server.
            connection: The socket connection to be handled.
            end_notify_func: The callback function to be called when this
                    thread is end.
            auto_close: A flag for whether this object should close the socket
                    at the end or not.
        '''
        super(_SocketConnectionHandler, self).__init__()
        self._delegator = _Delegator(backend)
        self._connection = connection
        self._stop_flag = False
        self._end_notify_func = end_notify_func
        self._logger = logging.getLogger(
                'ConnHandler for (%s, %d)' % self._connection.getpeername())
        self._auto_close = auto_close

        self._connection.settimeout(1)

    def run(self):
        '''Routine of this handler thread.'''
        try:
            while not self._stop_flag:
                (typee, buf) = self._recv_package()
                ret = self._delegator.handle_package(typee, buf)
                if ret is not None:
                    (typee, buf) = ret
                    self._send_package(typee, buf)
        except self._StopException as e:
            pass
        except Exception as e:
            self._logger.warning(utils.get_exception_msg(e))
            self._logger.info('Stop the handler.')
            self._end_notify_func(self._connection)
        finally:
            if self._auto_close:
                self._logger.info('Close the connection.')
                self._connection.close()

    def stop(self):
        '''Stops this thread.'''
        self._stop_flag = True

    def _recv_package(self):
        '''Receives a package from the socket connection.'''
        err = None
        try:
            buf = b''
            while len(buf) < net.PackageHeader.SIZE and not self._stop_flag:
                try:
                    a = self._connection.recv(
                            net.PackageHeader.SIZE - len(buf))
                    if not a:
                        raise _ProtocolError('cannot recv')
                    buf += a
                except socket.timeout as e:
                    pass
            (header, offs) = net.PackageHeader.load(buf)
            buf = b''
            while len(buf) < header.size and not self._stop_flag:
                try:
                    buf += self._connection.recv(header.size - len(buf))
                except socket.timeout as e:
                    pass
        except Exception as e:
            err = e
        if self._stop_flag:
            raise self._StopException()
        elif err is not None:
            raise err
        return (header.typee, buf)

    def _send_package(self, typee, buf):
        '''Sents a typed package content.

        Args:
            typee: Type of the package.
            buf: Byte array of the content.
        '''
        buf = net.PackageHeader(typee, len(buf)).dump() + buf
        total_sent = 0
        while buf and not self._stop_flag:
            curr_sent = self._connection.send(buf)
            if not curr_sent:
                raise _ProtocolError('cannot send')
            buf = buf[curr_sent : ]
            total_sent += curr_sent
        if self._stop_flag:
            raise self._StopException()

    class _StopException(Exception):
        '''An exception which will be raised if the stop flag become true.'''
        pass


class Server(object):
    '''A socket server services for the outcome query.

    Attributes:
        _backend: The real backend interface for proccessing the queryies.
        _bind_addr: Address to bind.
        _listen_port: Port to listen.
        _sock: References to a python socket object.
        _handler_threads: A set of threads which handling each connection.
        _logger: An instance of logging.Logger
        _stop_flag: A flag for whether the run main loop should stop or not.
    '''
    def __init__(self, backend, bind_addr, listen_port):
        '''Constructor.

        Args:
            backend: References to the real backend server interface.
            bind_addr: Address to bind.
            listen_port: Port to listen.
        '''
        self._backend = backend
        self._bind_addr = bind_addr
        self._listen_port = listen_port
        self._sock = None
        self._handler_threads = []
        self._stopped_threads = []
        self._logger = logging.getLogger('MinerProxyServer')
        self._stop_flag = False

    def run(self):
        '''Runs the server.

        Note that method will block and not return.
        '''
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.bind((self._bind_addr, self._listen_port))
        self._sock.listen(1024)
        self._sock.settimeout(1)
        while not self._stop_flag:
            try:
                (conn, addr) =  self._sock.accept()
            except socket.timeout as e:
                continue
            self._logger.info(
                    'Accept a connection from (%s, %d)' % conn.getpeername())
            thr = _SocketConnectionHandler(
                    self._backend, conn, self.notify_handler_thread_end)
            self._handler_threads.append(thr)
            thr.start()
        self._sock.close()

    # TODO(cathook): Re-writes below two functions to free resouces faster.
    def stop(self):
        '''Stops the server.'''
        self._stop_flag = True
        for handler_thread in self._handler_threads:
            handler_thread.stop()
            handler_thread.join()
        self._handler_threads = []
        self._logger.info('Close the server.')
        self._sock.close()
        self._sock = None

    def notify_handler_thread_end(self, thr):
        '''A callback function for notifying one of the thread handler is end.

        Args:
            thr: The stopped thread
        '''
        pass
