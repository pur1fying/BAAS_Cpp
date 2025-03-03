from flask import Flask, request

app = Flask(__name__)

@app.route('/hello', methods=['GET'])
def hello():
    return "Hello, World!"

@app.route('/test', methods=['POST'])
def post_data():
    data = request.json  # 如果发送的是 JSON 数据
    files = request.files
    print(files)
    print(data)

if __name__ == '__main__':
    app.run(host='localhost', port=1145)  # 监听所有 IP 地址，端口 5000
