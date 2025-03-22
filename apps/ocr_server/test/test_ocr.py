import json
import os
import random
import unittest

import cv2

from Client import client
from utils import count_files


class TestOcr(unittest.TestCase):

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

    def test_ocr(self):
        print("Test OCR")
        models = [
            "en-us",
            "ko-kr",
            "ja-jp",
            "zh-cn",
            "zh-cn_v3",
            "zh-tw",
            "ru-ru"
        ]

        post_file_ret_text_list = []
        shared_memory_ret_text_list = []

        test_image_path = os.path.join(os.path.dirname(__file__), "test_images", "ocr")
        # pass method shared memory
        print("<<< PASS METHOD : SHARE MEMORY >>>")
        ret = client.create_shared_memory("test", 1280 * 720 * 3)
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)

        for model in models:
            print(f"<<< {model} >>>")
            _dir = os.path.join(test_image_path, model)
            num_files = count_files(_dir)
            for i in range(0, num_files):
                image_path = os.path.join(test_image_path, model, f"{i}.png")
                img = cv2.imread(image_path)
                ret = client.ocr(
                    language=model,
                    origin_image=img,
                    candidates="",
                    pass_method=0,
                    ret_options=0b111,
                    local_path="",
                    shared_memory_name="test"
                )
                self.assertEqual(200, ret.status_code)
                j = json.loads(ret.text)
                time = j["time"]
                print(f"{i} time : [ {time} ms ]")
                # print(j["str_res"])
                shared_memory_ret_text_list.append(j["str_res"])

        ret = client.release_shared_memory("test")
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)

        # pass method post file
        print("<<< PASS METHOD : POST FILE >>>")
        for model in models:
            print(f"<<< {model} >>>")
            _dir = os.path.join(test_image_path, model)
            num_files = count_files(_dir)
            for i in range(0, num_files):
                image_path = os.path.join(test_image_path, model, f"{i}.png")
                img = cv2.imread(image_path)
                ret = client.ocr(
                    language=model,
                    origin_image=img,
                    candidates="",
                    pass_method=1,
                    ret_options=0b111,
                    local_path="",
                )
                self.assertEqual(200, ret.status_code)
                j = json.loads(ret.text)
                time = j["time"]
                print(f"{i} time : [ {time} ms ]")
                # print(j["str_res"])
                post_file_ret_text_list.append(j["str_res"])



        # same image same result
        self.assertEqual(len(post_file_ret_text_list), len(shared_memory_ret_text_list))
        for i in range(len(post_file_ret_text_list)):
            self.assertEqual(post_file_ret_text_list[i], shared_memory_ret_text_list[i])

    def test_ocr_bad_request(self):
        print("Test ocr_for_single_line_bad_request.")
        client.init_model("en-us", -1, 4, False)
        test_image_path = os.path.join(
            os.path.dirname(__file__),
            "test_images",
            "ocr",
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
                "language": True,  # error type
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
            {
                "language": "en-us",
                "pass_method": 2,
                "local_path": True,  # error type
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
        ret = client.enable_thread_pool(4)
        self.assertEqual(200, ret.status_code)

        ret = client.init_model(all_models, -1, 4, True)
        self.assertEqual(200, ret.status_code)
        j = json.loads(ret.text)
        self.assertEqual(expected_ret, j["ret"])
