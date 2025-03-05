import cv2
import requests
import os
import subprocess
import datetime
import shutil
import json
import psutil


class ServerConfig:
    def __init__(self):
        self.config = None
        self.host = None
        self.port = None
        self.base_url = None
        self.__init_config()
        self.kill_port_process()

    def __init_config(self):
        config_file_path = os.path.join(BaasOcrClient.server_folder_path, "config", "global_setting.json")
        with open(config_file_path, "r") as f:
            self.config = json.load(f)
            self.host = self.config["ocr"]["server"]["host"]
            self.port = self.config["ocr"]["server"]["port"]
            self.base_url = f"http://{self.host}:{self.port}"

    # kill existing server
    def kill_port_process(self):
        for proc in psutil.process_iter(['pid', 'name', 'connections']):
            for conn in proc.info['connections']:
                if conn.laddr.port == self.port:
                    proc.terminate()
                    return


class BaasOcrClient:
    server_folder_path = "C:/Users/pc/Desktop/work/c/BAAS_Cpp/cmake-build-release"
    executable_name = "BAAS_ocr_server.exe"

    def __init__(self):
        self.exe_path = os.path.join(self.server_folder_path, self.executable_name)
        if not os.path.exists(self.exe_path):
            raise Exception("Didn't find ocr server executable.")
        self.config = ServerConfig()
        self.server_process = None

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

    def start_server(self):
        if self.server_process is not None:
            return
        self.server_process = subprocess.Popen(
            self.exe_path,
            stdout=subprocess.PIPE
        )

    def stop_server(self):
        if self.server_process is not None:
            self.server_process.terminate()
            self.server_process = None

    def init_model(self, language: list[str], gpu_id=-1, num_thread=4):
        url = self.config.base_url + "/init_model"
        data = {
            "language": language,
            "gpu_id": gpu_id,
            "num_thread": num_thread
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
            pass_method: int = 1,
            local_path: str = "",
            ret_options: int = 0b100
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
        # TODO: shm pass method
        if pass_method == 0:
            return
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

    def ocr_for_single_line(self,
                            language: str,
                            origin_image=None,
                            candidates: str = "",
                            pass_method: int = 1,
                            local_path: str = "",
                            ret_options: int = 0b100
                            ):
        url = self.config.base_url + "/ocr_for_single_line"
        data = {
            "language": language,
            "candidates": candidates,
            "image": {
                "pass_method": pass_method,
            },
            "ret_options": ret_options
        }
        if pass_method == 0:
            return
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

    @staticmethod
    def get_image_bytes(image):
        _, encoded_image = cv2.imencode('.png', image)
        return encoded_image.tobytes()

    def get_text_box(self):
        pass


if __name__ == "__main__":
    client = BaasOcrClient()
    client.start_server()
