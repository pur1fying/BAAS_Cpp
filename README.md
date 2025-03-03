# BAAS_Cpp 
## BAAS_Cpp是一个安卓自动化脚本框架

1. 这是对[blue_archive_auto_script](https://github.com/pur1fying/blue_archive_auto_script)的C++重构 , 目的是实现一个功能齐全的自动化脚本框架,简化开发流程

2. 已经实现的功能(windows平台下):

### 配置管理: 使用json储存配置数据

对应代码 include/config

### 模拟器控制 (大部分灵感来自[Alas](https://github.com/LmeSzinc/AzurLaneAutoScript/tree/master/module/device) 是对python代码的C++翻译)

#### 模拟器截图方式: nemu_ipc, ascreencap, adb, scrcpy(有损)   

对应代码 include/device/screenshot

TODO: adb_nc, droidcast, ascreencap_nc

#### 模拟器控制方式: adb, scrcpy, nemu 

对应代码 include/device/control

TODO: maatouch, minitouch, hermit

### 特征识别系统:

简介:在我的理解下, 脚本的本质在于**检测某个特征(feature)进行对应操作(action)**, 所以BAAS框架在设计时就着重考虑了特征的多样性问题

include/feature 你可以继承BaseFeature类来定义任意形式的特征(无论是参数还是比较方式), 你可以使用opencv, ocr, 甚至更多的AI模型来判断图像中出现了哪些特征

注: 参数的任意性通过json实现, 如果你不了解json请先了解2.1中提及的仓库

ps:文档不全面, 请等待文档完善后使用这个框架

### 部署:

1. 单独开启一个仓库储存,编译好的文件 (一般放在gitee)

2. 然后修改installer.py 中的配置

3 .使用pyinstaller打包为exe即可

### APPS
1. BAAS_ocr_server: 一个简单的ocr服务器, 使用onnxruntime进行推理, 使用cpp-httplib进行http通信, 替代了原项目BAAS体积过大的OCR模块

Open Source Code used:
1. [json](https://github.com/nlohmann/json)
    - config management and data exchange
2. [opencv](https://github.com/opencv/opencv)
    - image processing and feature recognition
3. [onnxruntime](https://github.com/microsoft/onnxruntime)
    - OCR onnx model inference
4. [spdlog](https://github.com/gabime/spdlog)
    - log system
5. [RapidOcr-Onnx](https://github.com/RapidAI/RapidOcrOnnx)
    - OCR module
6. [cpp-httplib](https://github.com/yhirose/cpp-httplib)
    - OCR server
7. [ffmpeg](https://github.com/FFmpeg/FFmpeg)
    - scrcpy h264 decoding
8. [benchmark](https://github.com/google/benchmark)
    - performance test
9. [scrcpy](https://github.com/Genymobile/scrcpy)
    - device screenshot and control
10. [lz4](https://github.com/lz4/lz4)
    - ascreencap screenshot decod
11. [thread-pool](https://github.com/mtrebi/thread-pool)
    - thread pool
Thanks for all the open source code authors!