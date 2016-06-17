import cgi
from http.server import BaseHTTPRequestHandler
import json

from modules import main_handler


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
        super(HTTPRequestHandler, self).__init__(*args, **kwargs)

    def do_POST(self):
        if self.path != '/' or \
                self.headers['content-type'] != 'application/json':
            self.send_response(400)
            return
        length = int(self.headers['content-length'])
        data = self.rfile.read(length).decode('utf-8')
        try:
            obj = json.loads(data)
        except Exception as e:
            self.send_response(400)
            return
        try:
            ret = self._real_handler.handle_json(obj)
        except main_handler.MainHandlerError as e:
            self.send_response(400)
            return
        data = json.dumps(ret)
        self.send_response(200)
        self.send_header('content-type', 'application/json')
        self.end_headers()
        self.wfile.write(data.encode('utf-8'))
