#! /usr/bin/env python3

import argparse
import logging
import sys
import os
from modules.backend_interface import BackendInterface
from modules.server import Server
from modules.mining import BBSCrawler
from modules.build import BuildData

# import somethig

def main():
    logging.basicConfig(level=logging.INFO)
    logger = logging.getLogger()

    ap = argparse.ArgumentParser(description='A miner server.')

    ap.add_argument('--server_addr', type=str, nargs='?', default='localhost',
                    help='Address the server should bind to.')
    ap.add_argument('--server_port', type=int, nargs='?', default=8993,
                    help='The port the server listen to.')
    ap.add_argument('--board', type=str, nargs='?', default='Gossiping',
                    help='The Board you want to mine.')
    ap.add_argument('--number', type=int, nargs='?', default=2,
                    help='The Pages you want to mine.')
    ap.add_argument('--fetch_path', type=str, nargs='?', default='./',
                    help='The Place you want to put your mining file')
    ap.add_argument('--mining', type=bool, nargs='?', default=False,
                    help='Mining or not')
    opts = ap.parse_args()
    CurrentDir = os.getcwd()

    # TODO(joemen): Change it to the real backend server.
    if opts.mining == True:
        miner = BBSCrawler(opts.board,opts.number,opts.fetch_path)
        miner.getAllPagesInTheBoard()
        miner.getContent()
    os.chdir(CurrentDir)
    backend = BuildData(opts.fetch_path + opts.board)  #BackendInterface()
    os.chdir(CurrentDir)
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
