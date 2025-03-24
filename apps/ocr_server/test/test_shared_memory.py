import sys
import unittest
from Client import client
from utils import logger

class TestSharedMemory(unittest.TestCase):
    def setUp(self):
        logger.sub_title("Start Server")
        client.start_server()
        if not client.is_server_running():
            raise RuntimeError("Fail to start server.")

    def tearDown(self):
        logger.sub_title("Stop Server")
        client.stop_server()

    def test_shared_memory_bad_request(self):
        logger.hr("Test shared memory bad request.")
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
            logger.info(ret.text)
            self.assertEqual(400, ret.status_code)
            self.assertIn("Bad Request", ret.text)

