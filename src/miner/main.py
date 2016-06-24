#! /usr/bin/env python3

import argparse
import logging
import os
import sys

from modules.build import BuildData
from modules.l2_miner import L2Miner
from modules.mining import BBSCrawler
from modules.server import Server
from modules.utils import get_exception_msg


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
    ap.add_argument('--storage_path', type=str, nargs='?', default='./storage',
                    help='Path to storeage the mirror of the ptt server.')
    ap.add_argument('--cache_size', type=int, nargs='?', default=5000,
                    help='Number of documents should be cached in the RAM.')
    ap.add_argument('--miner2', type=bool, nargs='?', default=False,
                    help='Use the version 2 miner or not.')
    opts = ap.parse_args()

    if opts.miner2:
        backend = L2Miner(logging.getLogger('miner'),
                          opts.cache_size, opts.storage_path)
    else:
        current_dir = os.getcwd()
        if opts.mining == True:
            miner = BBSCrawler(opts.board,opts.number,opts.fetch_path)
            miner.getAllPagesInTheBoard()
            miner.getContent()
        os.chdir(current_dir)
        backend = BuildData(opts.fetch_path + opts.board)
        os.chdir(current_dir)

    server = Server(backend, opts.server_addr, opts.server_port)
    try:
        server.run()
    except KeyboardInterrupt as e:
        logger.info('Keyboard interrupt.')
    except SystemExit as _:
        logger.info('System exit.')
    except Exception as e:
        logger.info(get_exception_msg(e))
        sys.exit(1)
    finally:
        logger.info('Cleanup')
        server.stop()
    sys.exit(0)


if __name__ == '__main__':
    main()
