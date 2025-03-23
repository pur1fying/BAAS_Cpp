import json
import random
import unittest
from Client import client


class TestInitModel(unittest.TestCase):

    def setUp(self):
        print("-------------------------------------------------")
        print("Start server.")
        client.start_server()
        if not client.is_server_running():
            raise RuntimeError("Fail to start server.")

    def tearDown(self):
        print("Stop server.")
        client.stop_server()

    def test_init_model_bad_request(self):
        print("Test init model bad request.")
        request_data = [
            {
                "language": "en-us",
                "gpu_id": 0,
                "num_thread": 4,
                "EnableCpuMemoryArena": False
            },
            {
                "language": True,
                "gpu_id": 0,
                "num_thread": 4,
                "EnableCpuMemoryArena": False
            },
            {
                "language": 1,
                "gpu_id": 0,
                "num_thread": 4,
                "EnableCpuMemoryArena": True
            },
            {
                "language": {"en-us": True},
                "gpu_id": 0,
                "num_thread": 4,
                "EnableCpuMemoryArena": True
            },
            {
                "language": [1, 2],
                "gpu_id": 0,
                "num_thread": 4,
                "EnableCpuMemoryArena": True
            }
        ]
        for data in request_data:
            ret = client.init_model(
                data["language"],
                data["gpu_id"],
                data["num_thread"],
                data["EnableCpuMemoryArena"]
            )
            print(ret.text)
            self.assertEqual(400, ret.status_code)
            self.assertIn("Bad Request", ret.text)

    def test_init_single_model(self):
        print("Test init single model.")
        models = [
            "en-us",
            "ko-kr",
            "ja-jp",
            "zh-cn",
            "zh-cn_v3",
            "zh-tw",
            "ru-ru",
            "en-us",    # already initialized
            "non-exist-language"
        ]
        expected = [
            [1],
            [1],
            [1],
            [1],
            [1],
            [1],
            [1],
            [2],
            [0]
        ]
        for i in range(len(models)):
            ret = client.init_model([models[i]], -1, 4, False)
            self.assertEqual(200, ret.status_code)
            j = json.loads(ret.text)
            self.assertEqual(expected[i], j["ret"])

    def test_init_all_models(self):
        print("Test init all models.")
        all_models = [
            "en-us",
            "ko-kr",
            "ja-jp",
            "zh-cn",
            "zh-cn_v3",
            "zh-tw",
            "ru-ru"
        ]
        expected_ret = [1, 1, 1, 1, 1, 1, 1]
        random.shuffle(all_models)
        ret = client.init_model(all_models, -1, 4, False)
        self.assertEqual(200, ret.status_code)
        j = json.loads(ret.text)
        self.assertEqual(expected_ret, j["ret"])

    def test_release_all_models(self):
        print("Test release all models.")
        # no models are initialized
        ret = client.release_all()
        self.assertEqual(200, ret.status_code)

        # all models are initialized
        models = [
            "en-us",
            "ko-kr",
            "ja-jp",
            "zh-cn",
            "zh-cn_v3",
            "zh-tw",
            "ru-ru",
            "en-us",  # already released
            "non-exist-language"
        ]
        ret = client.init_model(models, -1, 4, False)
        self.assertEqual(200, ret.status_code)
        ret = client.release_all()
        print("ret" ,ret.text)
        self.assertEqual(200, ret.status_code)

    def test_release_single_model(self):
        print("Test release single model.")
        models = [
            "en-us",
            "ko-kr",
            "ja-jp",
            "zh-cn",
            "zh-cn_v3",
            "zh-tw",
            "ru-ru",
            "en-us",  # already released
            "non-exist-language"
        ]
        expected_ret = [
            [True],
            [True],
            [True],
            [True],
            [True],
            [True],
            [True],
            [False],
            [False]
        ]
        ret = client.init_model(models, -1, 4, False)
        self.assertEqual(200, ret.status_code)
        for i in range(len(models)):
            ret = client.release_model([models[i]])
            self.assertEqual(200, ret.status_code)
            j = json.loads(ret.text)
            self.assertEqual(expected_ret[i], j["ret"])