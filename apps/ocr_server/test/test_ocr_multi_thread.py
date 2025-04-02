import os
import cv2
import json
import time
import random
import unittest
import threading
from utils import logger
from Client import client
from utils import count_files


class TestOcr(unittest.TestCase):

    def setUp(self):
        logger.sub_title("Start Server")
        client.start_server()
        if not client.is_server_running():
            raise RuntimeError("Fail to start server.")
        self.init_all_models()

    def tearDown(self):
        logger.sub_title("Stop Server")
        client.stop_server()

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

        ret = client.init_model(all_models, -1, 1, True)
        self.assertEqual(200, ret.status_code)
        j = json.loads(ret.text)
        self.assertEqual(expected_ret, j["ret"])

    def test_multi_thread_ocr(self):
        logger.hr("Test Multi Thread OCR.")
        thread_count = 4
        image_cnt = 10

        language = "ja-jp"
        test_image_path = os.path.join(os.path.dirname(__file__), "test_images", "ocr", language)
        folder_image_count = count_files(test_image_path)

        # prepare share memory
        basic_shm_name = "test"
        for i in range(0, thread_count):
            shm_name = basic_shm_name + str(i)
            ret = client.create_shared_memory(shm_name, 1280 * 720 * 3)
            self.assertEqual(200, ret.status_code)
            self.assertEqual("Success.", ret.text)

        self.multi_thread_ret_text = dict()
        single_thread_ret_text = dict()

        # prepare image
        each_thread_test_image_name = []
        for i in range(0, thread_count):
            each_thread_test_image_name.append([])
            for j in range(0, image_cnt):
                # random select image
                random_index = random.randint(0, folder_image_count - 1)
                image_path = os.path.join(test_image_path, f"{random_index}.png")
                each_thread_test_image_name[i].append(image_path)

        logger.sub_title("Test Multi Thread OCR.")
        t_multi_thread_start = time.time()
        threads = []
        for i in range(0, thread_count):
            t = threading.Thread(target=self.ocr_thread, args=(i, each_thread_test_image_name[i], language))
            threads.append(t)
            t.start()

        for t in threads:
            t.join()
        t_multi_thread = time.time() - t_multi_thread_start
        logger.info(f"Multi thread OCR time : [ {t_multi_thread} s ]")

        # release share memory
        for i in range(0, thread_count):
            shm_name = basic_shm_name + str(i)
            ret = client.release_shared_memory(shm_name)
            self.assertEqual(200, ret.status_code)
            self.assertEqual("Success.", ret.text)

        client.create_shared_memory(basic_shm_name, 1280 * 720 * 3)
        # single thread ocr
        logger.sub_title("Test Single Thread OCR.")
        t_single_thread_start = time.time()
        for i in range(0, thread_count):
            for j in range(0, image_cnt):
                image_path = each_thread_test_image_name[i][j]
                img = cv2.imread(image_path)
                ret = client.ocr(
                    language=language,
                    origin_image=img,
                    candidates="",
                    pass_method=0,
                    ret_options=0b111,
                    local_path="",
                    shared_memory_name="test"
                )
                self.assertEqual(200, ret.status_code)
                j = json.loads(ret.text)
                t = j["time"]
                single_thread_ret_text[f"{i}_{image_path}"] = j["str_res"]
                logger.info(f"Image {image_path} | time {t} ms.")
        t_single_thread = time.time() - t_single_thread_start
        logger.info(f"Single thread OCR time : [ {t_single_thread} s ]")

        ret = client.release_shared_memory(basic_shm_name)
        self.assertEqual(200, ret.status_code)
        self.assertEqual("Success.", ret.text)

        self.assertLess(t_multi_thread, t_single_thread,
                        "Multi thread OCR time should be less than single thread OCR time.")

        # compare result
        for i in range(0, thread_count):
            for j in range(0, image_cnt):
                image_path = each_thread_test_image_name[i][j]
                self.assertEqual(self.multi_thread_ret_text[f"{i}_{image_path}"],
                                 single_thread_ret_text[f"{i}_{image_path}"],
                                 f"Thread {i} Image {image_path} result not equal.")

    def ocr_thread(self, thread_id, image_list, language):
        shm_name = "test" + str(thread_id)
        logger.info(f"Thread {thread_id} Start.")
        for image_path in image_list:
            img = cv2.imread(image_path)
            ret = client.ocr(
                language=language,
                origin_image=img,
                candidates="",
                pass_method=0,
                ret_options=0b111,
                local_path="",
                shared_memory_name=shm_name
            )
            self.assertEqual(200, ret.status_code)
            j = json.loads(ret.text)
            time = j["time"]
            self.multi_thread_ret_text[f"{thread_id}_{image_path}"] = j["str_res"]
            logger.info(f"Thread {thread_id} Image {image_path} | time {time} ms.")
        logger.info(f"OCR Thread {thread_id} Finished.")
