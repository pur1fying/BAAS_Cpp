import requests

def test_init_models():
    url = "http://localhost:1145/init_model"
    data = {
        "language": ["en-us", "ko-kr", "ja-jp", "zh-cn", "zh-tw", "ru-ru", "en-us", "non-exist-language"],
        "gpu_id": 0,
        "num_thread": 4
    }
    response = requests.post(url, json=data)
    print(response.text)


def test_release_all():
    url = "http://localhost:1145/release_all"
    response = requests.get(url)
    print(response.text)


if __name__ == "__main__":
    test_init_models()
    test_release_all()
