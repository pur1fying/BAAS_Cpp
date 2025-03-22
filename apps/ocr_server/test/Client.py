import os
import cv2
import sys
import json
import time
import socket
import shutil
import datetime
import requests
import subprocess
from multiprocessing import shared_memory


class ServerConfig:
    def __init__(self):
        self.config = None
        self.config_path = os.path.join(BaasOcrClient.server_folder_path, "config", "global_setting.json")
        self.host = None
        self.port = None
        self.server_is_remote = False
        self.base_url = None
        self.__init_config()

    def __init_config(self):
        if not os.path.exists(self.config_path):
            default_config_file_path = os.path.join(BaasOcrClient.server_folder_path, "resource", "global_setting.json")
            if not os.path.exists(default_config_file_path):
                raise Exception("Didn't find default config file.")
            os.mkdir(os.path.dirname(self.config_path))
            shutil.copy(default_config_file_path, self.config_path)
        with open(self.config_path, "r") as f:
            self.config = json.load(f)
            self.host = self.config["ocr"]["server"]["host"]
            self.port = self.config["ocr"]["server"]["port"]
            self.base_url = f"http://{self.host}:{self.port}"
            # check is remote
            if self.host != "localhost" and self.host != "127.0.0.1":
                self.server_is_remote = True


class BaasOcrClient:
    server_folder_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "build", "bin")
    executable_name = "BAAS_ocr_server"
    if sys.platform == "win32":
        executable_name += ".exe"

    def __init__(self):
        self.exe_path = os.path.join(self.server_folder_path, self.executable_name)
        if not os.path.exists(self.exe_path):
            raise Exception("Didn't find ocr server executable.")
        self.config = ServerConfig()
        self.server_process = None

    @staticmethod
    def kilL_existing_server():
        try:
            if sys.platform == "linux":
                subprocess.run(["pkill", "-9", BaasOcrClient.executable_name])
            elif sys.platform == "win32":
                subprocess.run(["taskkill", "/f", "/im", BaasOcrClient.executable_name], check=True)
        except Exception:
            pass

    # clear log since time_distance days ago
    def clear_log(self, time_distance=7):
        log_folder_path = os.path.join(self.server_folder_path, "output")
        if not os.path.exists(log_folder_path):
            return
        for name in os.listdir(log_folder_path):
            path = os.path.join(log_folder_path, name)
            if os.path.isdir(path):
                # name is yyyy-mm-dd_hh.mm.ss
                name = name.split("_")[0]
                year, month, day = map(int, name.split("-"))
                time_dis = (datetime.datetime.now() - datetime.datetime(year, month, day)).days
                if time_dis >= time_distance:
                    shutil.rmtree(path)

    def create_shared_memory(self, name, size):
        url = self.config.base_url + "/create_shared_memory"
        pass_name = "/" + name if sys.platform != "win32" else name
        data = {
            "shared_memory_name": pass_name,
            "size": size
        }
        ret = requests.post(url, json=data)
        if ret.status_code == 200:
            SharedMemory.get(name)
        return ret

    def release_shared_memory(self, name):
        url = self.config.base_url + "/release_shared_memory"
        pass_name = "/" + name if sys.platform != "win32" else name
        data = {
            "shared_memory_name": pass_name
        }
        SharedMemory.release(name)
        return requests.post(url, json=data)

    def enable_thread_pool(self, count=4):
        url = self.config.base_url + "/enable_thread_pool"
        data = {
            "thread_count": count
        }
        return requests.post(url, json=data)

    def disable_thread_pool(self):
        url = self.config.base_url + "/disable_thread_pool"
        return requests.post(url)

    def start_server(self):
        if self.server_process is not None:
            return
        # chmod +x BAAS_ocr_server
        if sys.platform == "linux":
            subprocess.run(["chmod", "+x", self.exe_path])
        self.server_process = subprocess.Popen(
            self.exe_path,
            cwd=self.server_folder_path,
            stdout=subprocess.DEVNULL
        )
        # wait for server start
        for _ in range(0, 30):
            try:
                requests.get(self.config.base_url)
                break
            except requests.exceptions.ConnectionError:
                time.sleep(0.1)

    def stop_server(self):
        url = self.config.base_url + "/shutdown"
        ret = requests.get(url)
        if ret.status_code == 200:
            self.server_process.wait(30)
            self.server_process = None
        return ret

    def init_model(self, language: list[str], gpu_id=-1, num_thread=4, EnableCpuMemoryArena=False):
        url = self.config.base_url + "/init_model"
        data = {
            "language": language,
            "gpu_id": gpu_id,
            "num_thread": num_thread,
            "EnableCpuMemoryArena": EnableCpuMemoryArena
        }
        return requests.post(url, json=data)

    def release_model(self, language: list[str]):
        url = self.config.base_url + "/release_model"
        data = {
            "language": language
        }
        return requests.post(url, json=data)

    def release_all(self):
        url = self.config.base_url + "/release_all"
        return requests.get(url)

    def ocr(self,
            language: str,
            origin_image=None,
            candidates: str = "",
            pass_method: int = 0,
            local_path: str = "",
            ret_options: int = 0b100,
            shared_memory_name: str = ""
            ):
        url = self.config.base_url + "/ocr"
        data = {
            "language": language,
            "candidates": candidates,
            "image": {
                "pass_method": pass_method,
            },
            "ret_options": ret_options
        }
        if pass_method == 0:
            if not SharedMemory.shm_exists(shared_memory_name):
                raise SharedMemoryError(f"Shared memory {shared_memory_name} not found, you should create it first.")
            col = origin_image.shape[1]
            row = origin_image.shape[0]
            size = col * row * 3
            SharedMemory.set_data(shared_memory_name, origin_image.tobytes(), size)
            data["image"]["shared_memory_name"] = "/" + shared_memory_name if sys.platform != "win32" else shared_memory_name
            data["image"]["shape"] = [row, col]
            return requests.post(url, json=data)
        if pass_method == 1:
            image_bytes = self.get_image_bytes(origin_image)
            files = {
                "data": (None, json.dumps(data), "application/json"),
                "image": ("image.png", image_bytes, "image/png")
            }
            return requests.post(url, files=files)
        if pass_method == 2:
            data["image"]["local_path"] = local_path
            return requests.post(url, json=data)

    def ocr_for_single_line(
                            self,
                            language: str,
                            origin_image=None,
                            candidates: str = "",
                            pass_method: int = 0,
                            local_path: str = "",
                            shared_memory_name: str = ""
                            ):
        url = self.config.base_url + "/ocr_for_single_line"
        data = {
            "language": language,
            "candidates": candidates,
            "image": {
                "pass_method": pass_method,
            },
        }
        if pass_method == 0:
            if not SharedMemory.shm_exists(shared_memory_name):
                raise SharedMemoryError(f"Shared memory {shared_memory_name} not found, you should create it first.")
            col = origin_image.shape[1]
            row = origin_image.shape[0]
            size = col * row * 3
            SharedMemory.set_data(shared_memory_name, origin_image.tobytes(), size)
            data["image"]["shared_memory_name"] = "/" + shared_memory_name if sys.platform != "win32" else shared_memory_name
            data["image"]["resolution"] = [col, row]
            return requests.post(url, json=data)
        elif pass_method == 1:
            image_bytes = self.get_image_bytes(origin_image)
            files = {
                "data": (None, json.dumps(data), "application/json"),
                "image": ("image.png", image_bytes, "image/png")
            }
            return requests.post(url, files=files)
        elif pass_method == 2:
            data["image"]["local_path"] = local_path
            return requests.post(url, json=data)
        else:
            raise OcrInternalError(f"Invalid pass_method {pass_method}")

    @staticmethod
    def get_image_bytes(image):
        _, encoded_image = cv2.imencode('.png', image)
        return encoded_image.tobytes()

    def get_text_box(self):
        pass

    def is_server_running(self, timeout=3):
        try:
            with socket.create_connection((self.config.host, self.config.port), timeout) as s:
                return True
        except ConnectionRefusedError:
            return False


class SharedMemory:
    shm_map = {}

    @staticmethod
    def get(name):
        if name not in SharedMemory.shm_map:
            SharedMemory.shm_map[name] = SharedMemory(name)
        return SharedMemory.shm_map[name]

    @staticmethod
    def shm_exists(name):
        return name in SharedMemory.shm_map

    @staticmethod
    def set_data(name, data, size):
        if name not in SharedMemory.shm_map:
            raise SharedMemoryError(f"Shared memory {name} not found")
        shm = SharedMemory.shm_map[name]
        if shm.size < size:
            raise SharedMemoryError(f"Shared memory {name} size {shm.size} not enough for {size}")
        shm.shm.buf[:size] = data

    @staticmethod
    def release(name):
        if name in SharedMemory.shm_map:
            SharedMemory.shm_map[name]._release()
            del SharedMemory.shm_map[name]

    def __init__(self, name):
        self.name = name
        self.size = None
        self.shm = None
        self._init()

    def _init(self):
        self.shm = shared_memory.SharedMemory(create=False, name=self.name)
        if self.shm is None:
            raise SharedMemoryError(f"Shared memory {self.name} not found")
        self.size = self.shm.size

    def _release(self):
        self.shm.close()
        self.shm.unlink()


class SharedMemoryError(Exception):
    """
        Shared memory error.
    """

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)

class OcrInternalError(Exception):
    """
        BAAS_ocr_server internal error
    """

    def __init__(self, message="Ocr Internal Error"):
        self.message = message
        super().__init__(self.message)

client = BaasOcrClient()
