import json
import os
import random
import unittest

import cv2

from Client import client
from utils import count_files, logger


class TestOcrForSingleLine(unittest.TestCase):

    def setUp(self):
        logger.sub_title("Start Server")
        client.start_server()
        if not client.is_server_running():
            raise RuntimeError("Fail to start server.")
        self.init_all_models()

    def tearDown(self):
        logger.sub_title("Stop Server")
        client.stop_server()

    def test_ocr_for_single_line(self):
        logger.hr("Test OCR for single line.")
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
            #     "0": "我愛愛麗絲",
            },
        }
        models = list(expected_results.keys())

        post_file_ret_text_list = []
        shared_memory_ret_text_list = []
        local_file_ret_text_list = []

        test_image_path = os.path.join(os.path.dirname(__file__), "test_images", "ocr_for_single_line")
        # pass method post file
        logger.sub_title("PASS METHOD : POST FILE")
        for model in models:
            logger.sub_title(model)
            _dir = os.path.join(test_image_path, model)
            num_files = count_files(_dir)
            for i in range(0, num_files):
                image_path = os.path.join(test_image_path, model, f"{i}.png")
                img = cv2.imread(image_path)
                ret = client.ocr_for_single_line(
                    language=model,
                    origin_image=img,
                    candidates="",
                    pass_method=1,
                    local_path="",
                )
                self.assertEqual(200, ret.status_code)
                j = json.loads(ret.text)
                time = j["time"]
                logger.info(f"{i} time : [ {time} ms ]")
                logger.info(j["text"])
                post_file_ret_text_list.append(j["text"])
                if str(i) in expected_results[model]:
                    self.assertEqual(expected_results[model][str(i)], j["text"])

        # pass method shared memory
        logger.sub_title("PASS METHOD : SHARED MEMORY")
        ret = client.create_shared_memory("test", 1280 * 720 * 3)
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)

        for model in models:
            logger.sub_title(model)
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
                logger.info(f"{i} time : [ {time} ms ]")
                logger.info(j["text"])
                shared_memory_ret_text_list.append(j["text"])
                if str(i) in expected_results[model]:
                    self.assertEqual(expected_results[model][str(i)], j["text"])

        ret = client.release_shared_memory("test")
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)
        
        # pass method local file
        logger.sub_title("PASS METHOD : LOCAL FILE")
        for model in models:
            logger.sub_title(model)
            _dir = os.path.join(test_image_path, model)
            num_files = count_files(_dir)
            for i in range(0, num_files):
                image_path = os.path.join(test_image_path, model, f"{i}.png")
                ret = client.ocr_for_single_line(
                    language=model,
                    origin_image=img,
                    candidates="",
                    pass_method=2,
                    local_path=image_path,
                )
                self.assertEqual(200, ret.status_code)
                j = json.loads(ret.text)
                time = j["time"]
                logger.info(f"{i} time : [ {time} ms ]")
                logger.info(j["text"])
                local_file_ret_text_list.append(j["text"])
                if str(i) in expected_results[model]:
                    self.assertEqual(expected_results[model][str(i)], j["text"])

        # same image same result
        self.assertEqual(len(post_file_ret_text_list), len(shared_memory_ret_text_list))
        self.assertEqual(len(post_file_ret_text_list), len(local_file_ret_text_list))
        for i in range(len(post_file_ret_text_list)):
            self.assertEqual(post_file_ret_text_list[i], shared_memory_ret_text_list[i])
            self.assertEqual(post_file_ret_text_list[i], local_file_ret_text_list[i])

    def test_ocr_for_single_line_bad_request(self):
        logger.hr("Test OCR for single line bad request.")
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
        ret = client.create_shared_memory(shm_name, 1280 * 720 * 3)
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)
        
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
            logger.info(ret.text)
            self.assertIn("Bad Request", ret.text)

        ret = client.release_shared_memory(shm_name)
        
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)

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
        self.assertEqual(ret.status_code, 200)
        self.assertEqual("Success.", ret.text)
        ret = client.init_model(all_models, -1, 4, False)
        self.assertEqual(ret.status_code, 200)
        j = json.loads(ret.text)
        self.assertEqual(j["ret"], expected_ret)
