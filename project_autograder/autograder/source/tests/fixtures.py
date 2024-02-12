import unittest
import logging
import time
import threading
import sys
import subprocess
import shutil
import os
import signal

import logging
log = logging.getLogger("CSE30264")

def read_shell_cmd(args, showdebug=False, **kwargs):
    # if showdebug:
    if True:
        log.debug(">>>>")
        log.debug("Starting: %s" % " ".join(args))

    p = subprocess.Popen(args, shell=False,
                         stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True, **kwargs)
    return p

class Worker(threading.Thread):
    def __init__(self, args):
        threading.Thread.__init__(self)
        self.args = args
        self.start()

    def run(self):
        try:
            self.process = read_shell_cmd(self.args)
            # log.error("PID: %d" % self.process.pid)

            (self.stdout, self.stderr) = self.process.communicate()
            self.retcode = self.process.returncode

            # log.debug("stdout: [%s]" % self.stdout)
            log.debug("stderr: [%s]" % self.stderr)
        except:

            log.debug("Failed to launch %s" % " ".join(self.args))
            self.stdout = None
            self.stderr = None
            self.retcode = None

    def killall(self, signals):
        for signal in signals:
            log.debug("SENDING SIGNAL %s to %s (%d)" % (signal, self.args[0], self.process.pid))
            try:
                os.kill(self.process.pid, signal)
            finally:
                return
            # except OSError, e:
                # log.error(e)

    # for large file transmission, probably 5 is too small
    def wait(self, max_time=5):
        if not self.isAlive():
            if not hasattr(self, 'retcode') or self.retcode is None:
                raise AssertionError("Failed to launch %s" % self.args[0])
            return

        self.join(max_time)

        if self.isAlive():
            self.killall([signal.SIGINT, signal.SIGTERM, signal.SIGQUIT])
            self.join(1)
            self.killall([signal.SIGKILL])
            self.join(1)
            # if self.isAlive():
            #     os.system('killall -INT -TERM -QUIT "%s"' % self.args[0])
            #     self.join(1)
            #     os.system('killall -KILL "%s"' % self.args[0])
            #     self.join(1)
            #     log.error("+++++++ FORKED PROCESS DID NOT DIE ++++++++++++")
            # raise AssertionError("Process %s didn't stop as expected" % " ".join(self.args))

        if not hasattr(self, 'retcode') or self.retcode is None:
            raise AssertionError("Failed to launch %s" % self.args[0])

class BasicTest(unittest.TestCase):
    """Base class for tests that don't require cleanups"""

    SUBMISSION = "/autograder/submission"
    REFERENCE = "/autograder/source/reference-implementation"
    HOSTNAME = "localhost"
    PORTNO = 5000

    FILE_1K   = "/autograder/file_1k"
    FILE_10K  = "/autograder/file_10k"
    FILE_100K = "/autograder/file_100k"
    FILE_1M   = "/autograder/file_1M"
    FILE_10M  = "/autograder/file_10M"
    LOG = logging.getLogger("CSE30264")

    def setUp(self):
        self._threads = []

    def tearDown(self):
        for thread in self._threads:
            if thread.isAlive():
                thread.killall([signal.SIGINT, signal.SIGTERM, signal.SIGQUIT, signal.SIGKILL])
                thread.join(2.0)

        self._threads = []

    def _start(self, args, nodelay=False):
        thread = Worker(args)
        self._threads.append(thread)
        if not nodelay:
            time.sleep(0.5) # there is no way to determine that app started, just hope it started within 1 seconds
        return thread

    def startSubmission(self, cmd, args, nodelay=False):
        updatedArgs = ["%s/%s" % (self.SUBMISSION, cmd)] + args
        return self._start(updatedArgs, nodelay)

    def startReference(self, cmd, args, nodelay=False):
        updatedArgs = ["%s/%s" % (self.REFERENCE, cmd)] + args
        return self._start(updatedArgs, nodelay)

    def runApp(self, cmd):
        p = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        (stdout, stderr) = p.communicate()
        self.LOG.debug("%s" % cmd)
        if stdout != '':
            self.LOG.debug("stdout: %s" % stdout)
        if stderr != '':
            self.LOG.debug("stderr: %s" % stderr)
        return (stdout, p.returncode)

class TestWithSavedData(BasicTest):
    """Base class for tests that use disk (will create folder and do cleanup afterwards)"""

    def setUp(self):
        BasicTest.setUp(self)
        self.relativeFolder = "/autograder/relative/"
        self.absoluteFolder = "/autograder/absolute/"
        # self.absoluteFolder = "%s/absolute/" % os.getcwd() # deleted to avoid permission issues
        shutil.rmtree(self.relativeFolder, ignore_errors = True)
        shutil.rmtree(self.absoluteFolder, ignore_errors = True)

        os.makedirs(self.relativeFolder)
        os.makedirs(self.absoluteFolder)

    def tearDown(self):
        # shutil.rmtree(self.relativeFolder, ignore_errors = True)
        # shutil.rmtree(self.absoluteFolder, ignore_errors = True)
        BasicTest.tearDown(self)
