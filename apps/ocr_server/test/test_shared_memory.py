import sys
import unittest
from Client import client


class TestSharedMemory(unittest.TestCase):
    def setUp(self):
        print("-------------------------------------------------")
        print("Start server.")
        client.start_server()
        if not client.is_server_running():
            raise RuntimeError("Fail to start server.")

    def tearDown(self):
        print("Stop server.")
        ret = client.stop_server()
        if ret.status_code != 200:
            raise RuntimeError("Fail to stop server.")
        self.assertEqual("Success.", ret.text)


    def test_shared_memory_bad_request(self):
        print("Test shared memory bad request.")
        request_data = [
            {
                "shared_memory_name": "/shm_name1",
                "size": -1
            },
            {
                "shared_memory_name": "/shm_name2",
                "size": 1.315251
            },
            {
                "shared_memory_name": "/shm_name3",
                "size": "1.315251"
            },
            {
                "shared_memory_name": "/shm_name4",
                "size": [1024]
            },
            {
                "shared_memory_name": "/shm_name5",
                "size": {
                    "size": 1024
                }
            },
            {
                "shared_memory_name": "/shm_name6",
                "size": True
            }
        ]
        for data in request_data:
            ret = client.create_shared_memory(
                name=data["shared_memory_name"],
                size=data["size"]
            )
            print(ret.text)
            self.assertEqual(400, ret.status_code)
            self.assertIn("Bad Request", ret.text)

