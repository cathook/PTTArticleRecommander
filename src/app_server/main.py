#! /usr/bin/env python3

import argparse
import logging
import sys

import modules.article_analysis
import modules.main_handler
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

    ap.add_argument('--article_analysis_server_addr', type=str, nargs='?',
                    default='localhost',
                    help='Address of the article analysis server')
    ap.add_argument('--article_analysis_server_port', type=int, nargs='?',
                    default=8997, help='Port of the article analysis server')
    ap.add_argument('--article_analysis_cache_size', type=int, nargs='?',
                    default=1000, help='Cached size of the miner proxy.')

    ap.add_argument('--echo_debug', type=bool, nargs='?',
                    default=False, help='Just become an echo server or not.')

    opts = ap.parse_args()

    try:
        if not opts.echo_debug:
            m = modules.miner.Miner(opts.miner_server_addr,
                                    opts.miner_server_port,
                                    opts.miner_cache_size,
                                    logger.getChild('Miner'))
            a = modules.article_analysis.ArticleAnalysis(
                    opts.article_analysis_server_addr,
                    opts.article_analysis_server_port,
                    opts.article_analysis_cache_size,
                    logger.getChild('ArticleAnalysis'))
            h = modules.main_handler.MainHandler(
                    m, a, logger.getChhild('MainHandler'))
        else:
            h = modules.main_handler.EchoMainHandler()
    except Exception as e:
        print('Exception: %r' % e)
        sys.exit(1)
    sys.exit(0)


if __name__ == '__main__':
    main()
