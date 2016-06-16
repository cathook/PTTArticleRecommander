#! /usr/bin/env python3

import argparse
import logging
import sys

import modules.miner


def main():
    logging.basicConfig(level=logging.INFO)
    logger = logging.getLogger()

    ap = argparse.ArgumentParser(description='The server side of the app')

    ap.add_argument('--miner_server_addr', type=str, nargs='?',
                    default='localhost', help='Address of the miner server')
    ap.add_argument('--miner_server_port', type=int, nargs='?',
                    default=8993, help='Port of the miner server')
    ap.add_argument('--miner_cache_size', type=int, nargs='?',
                    default=1000, help='Cached size of the miner proxy.')

    opts = ap.parse_args()

    try:
        m = modules.miner.Miner(opts.miner_server_addr,
                                opts.miner_server_port,
                                opts.miner_cache_size)
    except Exception as e:
        print('Exception: %r' % e)
        sys.exit(1)
    sys.exit(0)


if __name__ == '__main__':
    main()
