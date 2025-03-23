import unittest
from Client import client
import json


class TestThreadPool(unittest.TestCase):
    def setUp(self):
        print("-------------------------------------------------")
        print("Start server.")
        client.start_server()
        if not client.is_server_running():
            raise RuntimeError("Fail to start server.")

    def tearDown(self):
        print("Stop server.")
        client.stop_server()

    def test_thread_pool_bad_request(self):
        print("Test thread pool bad request.")
        request_data = [
            {
                "thread_count": -151
            },
            {
                "thread_count": 1.5
            },
            {
                "thread_count": "1"
            },
            {
                "thread_count": [1]
            },
            {
                "thread_count": {
                    "count": 1
                }
            }
        ]
        for data in request_data:
            ret = client.enable_thread_pool(data["thread_count"])
            print(ret.text)
            self.assertEqual(400, ret.status_code)
            self.assertIn("Bad Request", ret.text)

    def test_thread_pool_performance(self):
        print("Test thread pool performance.")
        all_models = [
            "en-us",
            "ko-kr",
            "ja-jp",
            "zh-cn",
            "zh-cn_v3",
            "zh-tw",
            "ru-ru",
        ]
        # disable thread pool
        ret = client.disable_thread_pool()
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)
        ret = client.init_model(all_models, -1, 4, False)
        self.assertEqual(200, ret.status_code)
        j = json.loads(ret.text)
        t_disable = j["time"]
        ret = client.release_all()
        self.assertEqual(200, ret.status_code)

        # enable thread pool
        thread_count = 4
        ret = client.enable_thread_pool(thread_count)
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)
        ret = client.init_model(all_models, -1, 4, False)
        self.assertEqual(200, ret.status_code)
        j = json.loads(ret.text)
        t_enable = j["time"]
        ret = client.release_all()
        self.assertEqual(200, ret.status_code)
        print(f"t Disable thread pool: {t_disable} ms")
        print(f"t Enable thread pool : {t_enable} ms")
        self.assertLess(t_enable, t_disable)
