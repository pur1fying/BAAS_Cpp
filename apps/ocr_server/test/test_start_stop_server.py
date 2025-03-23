import unittest
from Client import client


class TestStartStopServer(unittest.TestCase):
    def test_start_stop_server(self):
        # start
        client.start_server()
        self.assertTrue(client.is_server_running())
        # stop
        client.stop_server()
