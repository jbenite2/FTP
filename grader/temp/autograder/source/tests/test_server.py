import unittest
from gradescope_utils.autograder_utils.decorators import weight, tags, visibility
from .fixtures import BasicTest, TestWithSavedData

import os
import signal
import time

class ServerTests(TestWithSavedData):

    @classmethod
    def setUpClass(cls):
        cls.portOffset = 100

    @weight(5)
    @visibility("visible")
    def test_5(self, file=None, timeout=5, client='client'):
        """Test 5. Server accepts a connection and start saving a file"""
        if not file:
            file = self.FILE_1K

        ServerTests.portOffset += 1

        server = self.startSubmission('server', [str(self.PORTNO + ServerTests.portOffset), self.absoluteFolder])
        client = self.startReference(client, [self.HOSTNAME, str(self.PORTNO + ServerTests.portOffset), file])

        client.wait(timeout)
        self.assertEqual(client.isAlive(), False, "Reference client should have been able to finish transmission within %d seconds." % timeout)
        try:
            server.wait(5)
        except AssertionError:
            pass

        files = os.listdir(self.absoluteFolder)
        files.sort()

        self.LOG.debug("Content of output folder: %s" % files)
        self.assertEqual(files, ["1.file"], "Hints: 1) Server should have saved exactly one file, actual list of files: [%s]; 2) Check if the socket has been created and is actively listening on that specific port number; 3) Check if the file is correctly saved in the given path (e.g., if the path given to the server is /usr/bin/save then the file should be saved as /usr/bin/save/<CONNECTION-ID>.file)." % ", ".join(files))

    @weight(5)
    @visibility("visible")
    def test_9(self, file=None, timeout=5, client='client'):
        """Test 9. Server able to receive a small file (500 bytes) and save it in 1.file"""
        if not file:
            file = self.FILE_1K

        self.test_5(file=file, timeout=timeout, client=client)

        (diff, diffret) = self.runApp('diff "%s" "%s/1.file"' % (file, self.absoluteFolder))
        self.assertEqual(diffret, 0, "Hints: 1) Check if the file is correctly saved under the given path; 2) Check if the contents of the file sent and created are the same.")

    @weight(5)
    @visibility("visible")
    def test_10_1(self):
        """Test 10 (part 1). Server able to receive a medium file (1 MiB bytes, sent without delays) and save it in 1.file"""
        self.test_9(file=self.FILE_1M)

    @weight(5)
    @visibility("visible")
    def test_11_1(self):
        """Test 11 (part 1). Server able to receive a large file (100 MiB bytes, sent without delays)) and save it in 1.file"""
        self.test_9(file=self.FILE_10M, timeout=30)

    @weight(5)
    @visibility("visible")
    def test_10_2(self): #
        """Test 10 (part 2). Server able to receive a medium file (1 MiB bytes, sent with delays) and save it in 1.file"""
        self.test_9(file=self.FILE_10K, timeout=50, client='slow-client.py')

    @weight(5)
    @visibility("visible")
    def test_11_2(self): #
        """Test 11 (part 2). Server able to receive a large file (100 MiB bytes, sent with delays) and save it in 1.file"""
        self.test_9(file=self.FILE_100K, timeout=100, client='slow-client.py')

    @weight(5)
    @visibility("visible")
    def test_12_1(self, parallel=False):
        """Test 12. Server can properly receive 10 small files (sent without delays) in 1.file, 2.file, ... 10.file when a single client connects sequentially"""

        ServerTests.portOffset += 1

        server = self.startSubmission('server', [str(self.PORTNO + ServerTests.portOffset), self.absoluteFolder])

        files = [self.FILE_1K,
                 self.FILE_10K,
                 self.FILE_100K,
                 self.FILE_1M,
                 self.FILE_1M,
                 self.FILE_1M,
                 self.FILE_1M,
                 self.FILE_100K,
                 self.FILE_10K,
                 self.FILE_1K,
                ]
        fileSizes = {}
        for file in files:
            size = os.stat(file).st_size
            if size in fileSizes:
                fileSizes[size]['count'] += 1
            else:
                fileSizes[size] = {'file': file, 'count': 1}

        def runClient(file, parallel):
            client = self.startReference('client', [self.HOSTNAME, str(self.PORTNO + ServerTests.portOffset), file], nodelay=parallel)
            if not parallel:
                client.wait(10)
                self.assertEqual(client.isAlive(), False, "The reference client should have finished transmission.")
            return client

        clients = []
        for file in files:
            client = runClient(file=file, parallel=parallel)
            clients.append(client)

        if parallel:
            for client in clients:
                client.wait()
                self.assertEqual(client.isAlive(), False, "The reference client should have finished transmission")

        try:
            server.wait(10)
        except AssertionError, e:
            self.LOG.error(e)
            pass

        actualFiles = os.listdir(self.absoluteFolder)
        actualFiles.sort()

        self.LOG.debug("Content of output folder: %s" % actualFiles)
        expected = ["%d.file" % d for d in range(1,11)]
        expected.sort()
        self.assertEqual(actualFiles, expected, "Not expected number of saved files (expect 10, got the following: [%s])" % ", ".join(actualFiles))

        if not parallel:
            for index, file in enumerate(files):
                (diff, diffret) = self.runApp('diff "%s" "%s/%d.file"' % (file, self.absoluteFolder, index + 1))
                self.assertEqual(diffret, 0, "The saved file [%d.file] is different from the original [%s]" % (index + 1, file))
        else:
            # order not guaranteed, so just check that we have enough files of correct sizes
            for file in actualFiles:
                size = os.stat("%s/%s" % (self.absoluteFolder, file)).st_size
                self.assertEqual(size in fileSizes, True, "Incorrect file")

                origFile = fileSizes[size]['file']
                fileSizes[size]['count'] -= 1
                if fileSizes[size]['count'] == 0:
                    del fileSizes[size]

                (diff, diffret) = self.runApp('diff "%s" "%s/%s"' % (origFile, self.absoluteFolder, file))
                self.assertEqual(diffret, 0, "Saved and original file differ")


    @weight(5)
    @visibility("visible")
    def test_15(self):
        """Test 15. Server aborts connection (a file should be created, containing only ERROR string) when it doesn't receive data from client for more than 10 seconds"""

        ServerTests.portOffset += 1

        server = self.startSubmission('server', [str(self.PORTNO + ServerTests.portOffset), self.absoluteFolder])
        client = self.startReference('broken-client.py', [self.HOSTNAME, str(self.PORTNO + ServerTests.portOffset)])

        try:
            client.wait(15) # server suppose to abort the connection and write "ERROR" in the file
        except AssertionError:
            pass # client is broken, duh

        try:
            server.wait(1)
        except AssertionError:
            pass

        start = open("%s/1.file" % self.absoluteFolder).read(5)
        self.assertEqual(start, "ERROR", "Partial input should have been discraded and ERROR saved in file")
