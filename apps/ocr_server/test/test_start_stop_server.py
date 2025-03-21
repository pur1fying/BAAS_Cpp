import unittest
from Client import client


class TestStartStopServer(unittest.TestCase):
    def test_start_stop_server(self):
        # start
        client.start_server()
        self.assertTrue(client.is_server_running())
        # stop
        ret = client.stop_server()
        self.assertEqual(ret.status_code, 200)
        self.assertEqual(ret.text, "Success.")
