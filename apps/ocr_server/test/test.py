import json
import time
import mmap
import cv2
import requests
from multiprocessing.shared_memory import SharedMemory

basic_url = "http://localhost:1145/"


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
    image_path = 'apps/ocr_server/test/test_images/test_ocr1.png'
    image = cv2.imread(image_path)
    t1 = time.time()
    shm = SharedMemory(name="test_shared_memory", create=False)
    shm.buf[:image.size] = image.tobytes()
    t2 = time.time()
    print("Put image into shared memory: ", t2 - t1)
    data = {
        "language": "zh-cn",
        "image": {
            "pass_method": 0,
            "shared_memory_name": "test_shared_memory",
            "resolution": [image.shape[1], image.shape[0]],
        },
        "ret_options": 0b100
    }
    t1 = time.time()
    response = requests.post(url, json=data)
    t2 = time.time()
    print("Time: ", t2 - t1)
    ret = json.loads(response.text)
    print(json.dumps(ret, indent=4))


def test_ocr_for_single_line():
    url = basic_url + "ocr_for_single_line"
    test_img_path = "C:\\Users\\pc\\Desktop\\work\\c\\BAAS_Cpp\\cmake-build-debug\\resource\\ocr_models\\test_images\\en-us.png"
    image = cv2.imread(test_img_path)
    data = {
        "language": "en-us",
        "image": {
            "pass_method": 2,
            "local_path": test_img_path
        },
        "candidates": "I loveAri1s "
    }
    response = requests.post(url, json=data)
    print(json.dumps(json.loads(response.text), indent=4))


def test_create_shared_memory():
    url = basic_url + "create_shared_memory"
    data = {
        "shared_memory_name": "test_shared_memory",
        "size": 1280 * 720 * 3
    }
    t1 = time.time()
    response = requests.post(url, json=data)


def test_release_shared_memory():
    url = basic_url + "release_shared_memory"
    data = {
        "shared_memory_name": "test_shared_memory"
    }
    response = requests.post(url, json=data)


if __name__ == "__main__":
    test_enable_thread_pool()
    # test_disable_thread_pool()
    test_init_model()
    test_create_shared_memory()
    # test_release_shared_memory()

    test_ocr()
    # time.sleep(1)
    # test_ocr_for_single_line()
    # test_release_model()
    # test_release_all()
