import json
import time

import cv2
import requests

basic_url = "http://localhost:1145/"

image_path = 'apps/ocr_server/test/test_ocr3.png'


def test_enable_thread_pool(count=4):
    url = basic_url + "enable_thread_pool"
    data = {
        "thread_count": count
    }
    response = requests.post(url, json=data)
    print(response.text)


def test_disable_thread_pool():
    url = basic_url + "disable_thread_pool"
    response = requests.post(url)
    print(response.text)


def test_init_model():
    url = basic_url + "init_model"
    data = {
        "language": ["en-us", "ko-kr", "ja-jp", "zh-cn", "zh-cn_v3", "zh-tw", "ru-ru", "en-us", "non-exist-language"],
        "gpu_id": 0,
        "num_thread": 4
    }
    response = requests.post(url, json=data)
    print(response.text)


def test_release_all():
    url = basic_url + "release_all"
    response = requests.get(url)
    print(response.text)


def test_release_model():
    url = basic_url + "release_model"
    data = {
        "language": ["en-us", "ko-kr", "ja-jp", "zh-cn", "zh-cn_v3", "zh-tw", "ru-ru", "en-us", "non-exist-language"]
    }
    response = requests.post(url, json=data)
    print(response.text)


def test_ocr():
    url = basic_url + "ocr"
    image = cv2.imread(image_path)
    _, encoded_image = cv2.imencode('.png', image)
    image_bytes = encoded_image.tobytes()
    data = {
        "language": "zh-cn",
        "image": {
            "pass_method": 1,
        },
        "ret_options": 0b100

    }
    files = {
        "data": (None, json.dumps(data), "application/json"),
        "image": ("image.png", image_bytes, "image/png")
    }
    t1 = time.time()
    response = requests.post(url, files=files)
    t2 = time.time()
    print(t2 - t1)
    print(json.dumps(json.loads(response.text), indent=4))


def test_ocr_for_single_line():
    url = basic_url + "ocr_for_single_line"
    test_img_path = 'apps/ocr_server/test/test_images/test_ocr_for_single_line2.png'
    image = cv2.imread(test_img_path)
    _, encoded_image = cv2.imencode('.png', image)
    image_bytes = encoded_image.tobytes()
    data = {
        "language": "en-us",
        "image": {
            "pass_method": 1,
            "single_line": True
        },
        "candidates": "I loveAri1s "
    }
    files = {
        "data": (None, json.dumps(data), "application/json"),
        "image": ("image.png", image_bytes, "image/png")
    }
    response = requests.post(url, files=files)
    print(json.dumps(json.loads(response.text), indent=4))


if __name__ == "__main__":
    test_enable_thread_pool()
    # test_disable_thread_pool()
    test_init_model()
    # test_ocr()
    test_ocr_for_single_line()
    # test_release_model()
    # test_release_all()
