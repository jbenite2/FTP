#!/usr/bin/env python

import unittest
import logging
import sys
from gradescope_utils.autograder_utils.json_test_runner import JSONTestRunner
import tests

if __name__ == '__main__':
    logging.basicConfig(stream=sys.stderr)
    logging.getLogger("CSE30264").setLevel(logging.DEBUG)
    if len(sys.argv) > 1:
        suite = unittest.defaultTestLoader.loadTestsFromNames(sys.argv[1:], tests)
    else:
        suite = unittest.defaultTestLoader.loadTestsFromModule(tests)
    JSONTestRunner().run(suite)
