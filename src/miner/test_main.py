#! /usr/bin/env python3

import sys
import unittest


def main():
    tests = unittest.TestLoader().discover('.', pattern='*_test.py')
    result = unittest.runner.TextTestRunner().run(tests)
    sys.exit(0 if result.wasSuccessful() else 1)

if __name__ == '__main__':
    main()
