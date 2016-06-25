#! /usr/bin/env python3

import argparse
from http.server import HTTPServer
import logging
import os
import ssl
import sys

import modules.article_analysis
import modules.http_request_handler
import modules.main_handler
import modules.miner
from modules.utils import get_exception_msg


PREFIX_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), '../..')
SHARE_PATH = os.path.join(PREFIX_PATH, 'share/app_server')


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

    ap.add_argument('--server_addr', type=str, nargs='?',
                    default='localhost', help='Server bind address.')
    ap.add_argument('--server_port', type=int, nargs='?',
                    default=8085, help='Server bind port.')

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
                    m, a, logger.getChild('MainHandler'))
        else:
            h = modules.main_handler.EchoMainHandler()

        def handler(*args, **kwargs):
            return modules.http_request_handler.HTTPRequestHandler(
                    h, logger.getChild('HTTPRequestHandler'), *args, **kwargs)
        s = HTTPServer((opts.server_addr, opts.server_port), handler)

        s.socket = ssl.wrap_socket(
                s.socket,
                keyfile=os.path.join(SHARE_PATH, 'key.pem'),
                certfile=os.path.join(SHARE_PATH,'cert.pem'),
                server_side=True)

        logger.info('Starts the server.')

        s.serve_forever()
    except KeyboardInterrupt as e:
        logger.info('Keyboard interrupt.')
    except SystemExit as _:
        logger.info('System exit.')
    except Exception as e:
        logger.error(get_exception_msg(e))
        sys.exit(1)
    finally:
        try:
            s.server_close()
        except Exception as _:
            pass
    sys.exit(0)


if __name__ == '__main__':
    main()
