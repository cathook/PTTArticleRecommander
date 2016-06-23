#! /usr/bin/env python3

import sys
import unittest


def main():
    pattern = '*_test.py'
    if len(sys.argv) > 1:
        pattern = sys.argv[1]
    tests = unittest.TestLoader().discover('.', pattern=pattern)
    result = unittest.runner.TextTestRunner().run(tests)
    sys.exit(0 if result.wasSuccessful() else 1)

if __name__ == '__main__':
    main()
