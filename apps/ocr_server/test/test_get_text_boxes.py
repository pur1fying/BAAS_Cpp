import json
import os
import cv2
import unittest
from Client import client
from utils import count_files, logger


class TestGetTextBoxes(unittest.TestCase):

    def setUp(self):
        logger.sub_title("Start Server")
        client.start_server()
        if not client.is_server_running():
            raise RuntimeError("Fail to start server.")

    def tearDown(self):
        logger.sub_title("Stop Server")
        client.stop_server()
        
    def test_get_text_boxes(self):
        logger.hr("Test Get Text Boxes.")
        client.init_model(["en-us"], -1, 4, False)
        expected_box_count = {
            "en-us":[11, 11, 28, 117, 66, 32, 37],
            "ja-jp":[5, 11, 10, 9, 10, 32, 48, 34, 21, 59, 66, 64, 63],
            "ko-kr":[40, 21, 7, 9, 6, 64, 29, 48, 10],
            "ru-ru":[23, 42, 26, 51],
            "zh-cn":[44, 13, 29, 25, 41, 25],
            "zh-cn_v3":[56, 52, 20, 28, 30],
            "zh-tw":[28, 10, 8, 10, 20, 14]
        }
        models = list(expected_box_count.keys())
        
        shared_memory_ret_box_count_list = []
        post_file_ret_box_count_list = []
        local_file_ret_box_count_list = []

        test_image_path = os.path.join(os.path.dirname(__file__), "test_images", "ocr")
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
                ret = client.get_text_boxes(
                    language="en-us",
                    origin_image=img,
                    pass_method=0,
                    local_path="",
                    shared_memory_name="test"
                )
                self.assertEqual(200, ret.status_code)
                j = json.loads(ret.text)
                time = j["time"]
                logger.info(f"{i} time : [ {time} ms ]")
                shared_memory_ret_box_count_list.append(len(j["text_boxes"]))
                self.assertEqual(len(j["text_boxes"]), expected_box_count[model][i])

        ret = client.release_shared_memory("test")
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)

        # pass method post file
        logger.sub_title("PASS METHOD : POST FILE")
        for model in models:
            logger.sub_title(model)
            _dir = os.path.join(test_image_path, model)
            num_files = count_files(_dir)
            for i in range(0, num_files):
                image_path = os.path.join(test_image_path, model, f"{i}.png")
                img = cv2.imread(image_path)
                ret = client.get_text_boxes(
                    language="en-us",
                    origin_image=img,
                    pass_method=1,
                    local_path="",
                )
                self.assertEqual(200, ret.status_code)
                j = json.loads(ret.text)
                time = j["time"]
                logger.info(f"{i} time : [ {time} ms ]")
                post_file_ret_box_count_list.append(len(j["text_boxes"]))

        # pass method local file
        logger.sub_title("PASS METHOD : LOCAL FILE")
        for model in models:
            logger.sub_title(model)
            _dir = os.path.join(test_image_path, model)
            num_files = count_files(_dir)
            for i in range(0, num_files):
                image_path = os.path.join(test_image_path, model, f"{i}.png")
                img = cv2.imread(image_path)
                ret = client.get_text_boxes(
                    language="en-us",
                    origin_image=img,
                    pass_method=2,
                    local_path=image_path,
                )
                self.assertEqual(200, ret.status_code)
                j = json.loads(ret.text)
                time = j["time"]
                logger.info(f"{i} time : [ {time} ms ]")
                local_file_ret_box_count_list.append(len(j["text_boxes"]))

        # same image same result
        self.assertEqual(len(post_file_ret_box_count_list), len(local_file_ret_box_count_list))
        self.assertEqual(len(post_file_ret_box_count_list), len(shared_memory_ret_box_count_list))
        for i in range(len(post_file_ret_box_count_list)):
            self.assertEqual(post_file_ret_box_count_list[i], local_file_ret_box_count_list[i])
            self.assertEqual(post_file_ret_box_count_list[i], shared_memory_ret_box_count_list[i])


    def test_get_text_boxes_bad_request(self):
        logger.hr("Test Get Text Boxes Bad Request.")
        client.init_model(["en-us"], -1, 4, False)
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
            ret = client.get_text_boxes(
                language=data["language"],
                origin_image=img,
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
