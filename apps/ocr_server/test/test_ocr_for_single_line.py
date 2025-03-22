import json
import os
import random
import unittest

import cv2

from Client import client
from utils import count_files


class TestOcrForSingleLine(unittest.TestCase):

    def setUp(self):
        print("Start server.")
        client.start_server()
        if not client.is_server_running():
            raise RuntimeError("Fail to start server.")
        self.init_all_models()

    def tearDown(self):
        print("Stop server.")
        ret = client.stop_server()
        if ret.status_code != 200:
            raise RuntimeError("Fail to stop server.")
        self.assertEqual("Success.", ret.text)

    def test_ocr_for_single_line(self):
        print("Test ocr_for_single_line.")

        expected_results = {
            "en-us": {
                "0": "I love Aris",
                "2": "How are you today?",
                "3": "I enjoy listening to music.",
                "4": "Where do you live?",
                "5": "I'm learning English. "

            },
            "ja-jp": {
                "0": "アリスが大好きです",
                "2": "確認しました"
            },
            "ko-kr": {
                "0": "나는엘리스를사랑한대"

            },
            "ru-ru": {
                "0": "ялюблюэлис"
            },
            "zh-cn": {
                "0": "我爱爱丽丝",
            },
            "zh-cn_v3": {
                "0": "我爱爱丽丝",
            },
            "zh-tw": {
                "0": "我愛愛麗絲",
            },
        }
        models = list(expected_results.keys())
        client.init_model(models, -1, 4, False)

        test_image_path = os.path.join(os.path.dirname(__file__), "test_images", "ocr_for_single_line")
        # pass method post file

        # pass method shared memory
        ret = client.create_shared_memory("test", 1280 * 720 * 3)
        self.assertEqual(200, ret.status_code)
        for model in models:
            print(f"<<< {model} >>>")
            _dir = os.path.join(test_image_path, model)
            num_files = count_files(_dir)
            for i in range(0, num_files):
                image_path = os.path.join(test_image_path, model, f"{i}.png")
                img = cv2.imread(image_path)
                ret = client.ocr_for_single_line(
                    language=model,
                    origin_image=img,
                    candidates="",
                    pass_method=0,
                    local_path="",
                    shared_memory_name="test"
                )
                self.assertEqual(200, ret.status_code)
                j = json.loads(ret.text)
                time = j["time"]
                print(f"{i} time : [ {time} ms ]")
                print(j["text"])
                if str(i) in expected_results[model]:
                    self.assertEqual(expected_results[model][str(i)], j["text"])

        client.release_shared_memory("test")
        self.assertEqual(200, ret.status_code)

    def test_ocr_for_single_line_bad_request(self):
        print("Test ocr_for_single_line_bad_request.")
        client.init_model("en-us", -1, 4, False)
        test_image_path = os.path.join(
            os.path.dirname(__file__),
            "test_images",
            "ocr_for_single_line",
            "en-us",
            "0.png"
        )
        img = cv2.imread(test_image_path)
        shm_name = "test"
        client.create_shared_memory(shm_name, 1280 * 720 * 3)
        request_data = [
            {
                "language": "non-exist-language",
                "pass_method": 0,
                "local_path": "",
                "shared_memory_name": shm_name
            },
            {
                "language": ["en-us", "ko-kr"],  # error type
                "pass_method": 0,
                "local_path": "",
                "shared_memory_name": shm_name
            },
            {
                "language": 114514,  # error type
                "pass_method": 0,
                "local_path": "",
                "shared_memory_name": shm_name
            },
            {
                "language": {
                    "language": "en-us"  # error type
                },
                "pass_method": 0,
                "local_path": "",
                "shared_memory_name": shm_name
            },
            {
                "language": "en-us",
                "pass_method": 2,
                "local_path": "non-exist-path",
                "shared_memory_name": shm_name
            },
            {
                "language": "en-us",
                "pass_method": 2,
                "local_path": ["non-exist-path"],  # error type
                "shared_memory_name": shm_name
            },
            {
                "language": "en-us",
                "pass_method": 2,
                "local_path": {
                    "path": "non-exist-path"  # error type
                },
                "shared_memory_name": shm_name
            },
            {
                "language": "en-us",
                "pass_method": 2,
                "local_path": 114514,  # error type
                "shared_memory_name": shm_name
            },
        ]
        for data in request_data:
            ret = client.ocr_for_single_line(
                language=data["language"],
                origin_image=img,
                candidates="",
                pass_method=data["pass_method"],
                local_path=data["local_path"],
                shared_memory_name=data["shared_memory_name"]
            )
            self.assertEqual(400, ret.status_code)
            print(ret.text)
            self.assertIn("Bad Request", ret.text)

    def init_all_models(self):
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
        self.assertEqual(ret.status_code, 200)
        j = json.loads(ret.text)
        self.assertEqual(j["ret"], expected_ret)
