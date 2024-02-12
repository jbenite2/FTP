import unittest
from gradescope_utils.autograder_utils.decorators import weight, tags, visibility
from .fixtures import BasicTest, TestWithSavedData

import os
import signal

class ClientTests(TestWithSavedData):

    @classmethod
    def setUpClass(cls):
        cls.portOffset = 0

    @weight(5)
    @visibility("visible")
    def test_4(self, file=None):
        """Test 4. Client connects and starts transmitting a file"""
        if not file:
            file = self.FILE_1K

        ClientTests.portOffset += 1

        server = self.startReference('server', [str(self.PORTNO + ClientTests.portOffset), self.absoluteFolder])
        client = self.startSubmission('client', [self.HOSTNAME, str(self.PORTNO + ClientTests.portOffset), file])

        client.wait()
        self.assertEqual(client.isAlive(), False, "")
        self.assertEqual(client.retcode, 0, "Client exit code is not zero (%d, stderr: %s)" % (client.retcode, client.stderr))

        try:
            server.wait(1)
        except AssertionError:
            pass

        files = os.listdir(self.absoluteFolder)
        files.sort()

        self.LOG.debug("Content of output folder: %s" % files)
        self.assertEqual(files, ["1.file"], "Hints: 1) Check if the <FILE-DIR>/<CONNECTION-ID>.file has been created; 2) Check if the client has connected to the correct port and is sending data to the server.")

    @weight(5)
    @visibility("visible")
    def test_6(self, file=None):
        """Test 6. Client able to successfully transmit a small file (500 bytes)"""
        if not file:
            file = self.FILE_1K

        self.test_4(file=file)

        (diff, diffret) = self.runApp('diff "%s" "%s/1.file"' % (file, self.absoluteFolder))
        self.assertEqual(diffret, 0, "Hints: 1) Check if the <FILE-DIR>/<CONNECTION-ID>.file has been created;  2) Check if the client exits with code 0; 3) Check if the contents of the file sent and created are the same.")

    @weight(5)
    @visibility("visible")
    def test_7(self):
        """Test 7. Client able to successfully transmit a medium size file (1 MiB)"""
        self.test_6(file=self.FILE_1M)

    @weight(5)
    @visibility("visible")
    def test_8(self):
        """Test 8. Client able to successfully transmit a large size file (100 MiB)"""
        self.test_6(file=self.FILE_10M)

    @weight(5)
    @visibility("visible")
    def test_13(self):
        """Test 13. Client handles abort connection attempt after 10 seconds"""

        ClientTests.portOffset += 1

        client = self.startSubmission('client', ["1.1.1.1", str(self.PORTNO), self.FILE_1K])
        client.wait(25) # should have aborted the connection
        self.assertEqual(client.isAlive(), False, "Client should have aborted connection after 10 seconds")
        self.assertNotEqual(client.retcode, 0, "Client exit code should have been non-zero")
        # self.assertEqual(client.stderr[0:5], "ERROR", "Client's stderr should have started with 'ERROR:'");
        self.LOG.debug("Content of output folder: %s" % client.stderr.split('\n'))
        parsed_stderr = client.stderr.split('\n')
        if parsed_stderr[-1] == '':
            del parsed_stderr[-1]
        self.assertEqual(parsed_stderr[-1][0:5], "ERROR", "Client's stderr should have started with 'ERROR:'");

    @weight(5)
    @visibility("visible")
    def test_14(self):
        """Test 14. Client aborts connection when server gets disconnected (server app or network connection is down)"""

        ClientTests.portOffset += 1

        server = self.startReference('broken-server.py', [str(self.PORTNO + ClientTests.portOffset)])
        client = self.startSubmission('client', [self.HOSTNAME, str(self.PORTNO + ClientTests.portOffset), self.FILE_10M])
        client.wait(25) # should have aborted the connection, as server not accepting data
        parsed_stderr = client.stderr.split('\n')
        if parsed_stderr[-1] == '':
            del parsed_stderr[-1]
        self.assertEqual(parsed_stderr[-1][0:5], "ERROR", "Client's stderr should have started with 'ERROR:'");

        try:
            server.wait(1)
        except AssertionError:
            pass
