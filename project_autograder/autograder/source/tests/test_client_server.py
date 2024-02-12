import unittest
from gradescope_utils.autograder_utils.decorators import weight, tags, visibility
from .fixtures import BasicTest, TestWithSavedData

import os
import signal
import time

class ClientServerTests(TestWithSavedData):

    @classmethod
    def setUpClass(cls):
        cls.portOffset = 300

    def setUp(self):
        TestWithSavedData.setUp(self)
        self.runApp('tc qdisc add dev lo root netem loss 5% delay 100ms')

    def tearDown(self):
        self.runApp('tc qdisc del dev lo root')
        TestWithSavedData.tearDown(self)

    @weight(10)
    @visibility("visible")
    def test_16_1(self, parallel=False):
        """Test 16. Client is able to successfully send and server is able to properly receive and save 10 large files over lossy and large delay network (we will use tc based emulation)."""

        ClientServerTests.portOffset += 1

        server = self.startSubmission('server', [str(self.PORTNO + ClientServerTests.portOffset), self.absoluteFolder])

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
            client = self.startReference('client', [self.HOSTNAME, str(self.PORTNO + ClientServerTests.portOffset), file], nodelay=parallel)
            if not parallel:
                client.wait()
                self.assertEqual(client.isAlive(), False, "The reference client should have finished transmission")
            return client

        clients = []
        for file in files:
            client = runClient(file=file, parallel=parallel)
            clients.append(client)

        if parallel:
            for client in clients:
                client.wait(10)
                self.assertEqual(client.isAlive(), False, "The reference client should have finished transmission")

        try:
            server.wait(10)
        except AssertionError:
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

    # @weight(2.5)
    # @visibility("after_published")
    # def test_16_2(self):
    #     """Test 16 (part 2). Client able to successfully send and server properly receive and save large file over lossy and large delay network (we will use tc based emulation)."""
    #
    #     self.test_17_1(parallel=True)
