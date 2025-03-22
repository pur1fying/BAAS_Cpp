import unittest
from Client import client


class TestStartStopServer(unittest.TestCase):
    def test_start_stop_server(self):
        # start
        client.start_server()
        self.assertTrue(client.is_server_running())
        # stop
        ret = client.stop_server()
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)
