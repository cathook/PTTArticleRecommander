import cgi
from http.server import BaseHTTPRequestHandler
import json

from modules import main_handler
from modules.utils import get_exception_msg


class HTTPRequestHandler(BaseHTTPRequestHandler):
    '''The http request handler.

    Attributes:
        _real_handler: Real handler.
        _logger: The logger.
    '''
    def __init__(self, real_handler, logger, *args, **kwargs):
        '''Constructor.

        Args:
            real_handler: Real handler.
            logger: The logger.
        '''
        self._real_handler = real_handler
        self._logger = logger
        super(HTTPRequestHandler, self).__init__(*args, **kwargs)

    def do_POST(self):
        if self.path != '/' or \
                self.headers['content-type'] != 'application/json':
            self._logger.warning('Bad url path or content-type.')
            self.send_response(400)
            return
        try:
            length = int(self.headers['content-length'])
            data = self.rfile.read(length).decode('utf-8')
            obj = json.loads(data)
            ret = self._real_handler.handle_json(obj)
            data = json.dumps(ret).encode('utf-8')
        except Exception as e:
            self._logger.warning(get_exception_msg(e))
            self.send_response(400)
            return
        self.send_response(200)
        self.send_header('content-type', 'application/json')
        self.end_headers()
        self.wfile.write(data)
