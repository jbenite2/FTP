import unittest
from gradescope_utils.autograder_utils.decorators import weight, tags, visibility
from .fixtures import BasicTest, TestWithSavedData

import subprocess
import signal

class MiscChecks(BasicTest):
    # @weight(2.5)
    # @visibility("visible")
    # def test_1(self):
    #     """1. At least 3 git commits"""
    #     (stdout, retcode) = self.runApp('git -C "/autograder/submission" log --pretty=format:"%h - %ai: (%an <%ae>) %s"')
    #     self.assertGreaterEqual(len(stdout.splitlines()), 0, "At least 3 git commits are expected")

    @weight(1.25)
    @visibility("visible")
    def test_1_1(self):
        """Test 1 (part 1). Client handles incorrect non-existing hostname"""

        process = self.startSubmission('client', ['abc', str(self.PORTNO), self.FILE_1K])
        process.wait()
        self.assertNotEqual(process.retcode, 0, "Hints: 1) Client should have returned a non-zero exit code; 2) For the hostname, ensure that you are using the correct hostname/IP address of the server (e.g., localhost or 127.0.0.1)")
        parsed_stderr = process.stderr.split('\n')
        if parsed_stderr[-1] == '':
            del parsed_stderr[-1]
        self.assertEqual(parsed_stderr[-1].startswith("ERROR:"), True, "stderr should have started with ERROR: (%s)" % process.stderr)

    @weight(1.25)
    @visibility("visible")
    def test_1_2(self):
        """Test 1 (part 2). Client handles incorrect port"""
        process = self.startSubmission('client', [self.HOSTNAME, "-1", self.FILE_1K])
        process.wait()
        self.assertNotEqual(process.retcode, 0, "Hints: 1) Client should have returned a non-zero exit code; 2) Check if the given port number is within the range (range of reserved ports is 0-1023).")
        parsed_stderr = process.stderr.split('\n')
        if parsed_stderr[-1] == '':
            del parsed_stderr[-1]
        self.assertEqual(parsed_stderr[-1].startswith("ERROR:"), True, "stderr should have started with ERROR: (%s)" % process.stderr)

    @weight(2.5)
    @visibility("visible")
    def test_2(self):
        """Test 2. Server handles incorrect port"""
        process = self.startSubmission('server', ['-1', '/tmp'])
        process.wait()
        self.assertNotEqual(process.retcode, 0, "Hints: 1) Server should have returned an non-zero exit code; 2) Check if the given port number is within the range (range of reserved ports is 0-1023).")
        parsed_stderr = process.stderr.split('\n')
        if parsed_stderr[-1] == '':
            del parsed_stderr[-1]
        self.assertEqual(parsed_stderr[-1].startswith("ERROR:"), True, "stderr should have started with ERROR: (%s)" % process.stderr)

    @weight(5.0)
    @visibility("visible")
    def test_3(self):
        """Test 3. Server handles SIGTERM / SIGQUIT signals"""
        process = self.startSubmission('server', [str(self.PORTNO), '/tmp'])
        process.killall([signal.SIGINT, signal.SIGTERM, signal.SIGQUIT])
        process.wait()
        self.assertEqual(process.isAlive(), False, "Hints: 1) Make sure to register signal handlers for SIGTERM and SIGQUIT using <signal.h>; 2)Check if the exit code is 0 when the server receives the SIGQUIT/SIGTERM signals.")
