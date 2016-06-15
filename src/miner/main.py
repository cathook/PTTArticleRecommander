#! /usr/bin/env python3

import argparse
import logging
import sys

from modules.backend_interface import BackendInterface
from modules.server import Server


def main():
    logging.basicConfig(level=logging.INFO)
    logger = logging.getLogger()

    ap = argparse.ArgumentParser(description='A miner server.')

    ap.add_argument('--server_addr', type=str, nargs='?', default='localhost',
                    help='Address the server should bind to.')
    ap.add_argument('--server_port', type=int, nargs='?', default=8993,
                    help='The port the server listen to.')

    opts = ap.parse_args()

    # TODO(joemen): Change it to the real backend server.
    backend = BackendInterface()
    server = Server(backend, opts.server_addr, opts.server_port)
    try:
        server.run()
    except Exception as e:
        server.stop()
        logger.info('Get exception: %r' % e)
        sys.exit(1)
    sys.exit(0)


if __name__ == '__main__':
    main()
