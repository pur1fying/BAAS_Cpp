# BAAS_Cpp 
## BAAS_Cpp是一个安卓自动化脚本框架

1.这是对blue_archive_auto_script的C++重构 (https://github.com/pur1fying/blue_archive_auto_script), 目的是实现一个功能齐全的自动化脚本框架,简化开发流程

2.已经实现的功能(windows平台下):

2.1 配置管理: 使用json储存配置数据(https://github.com/nlohmann/json) 

对应代码 include/config

2.2 模拟器控制 (大部分灵感来自Alas (https://github.com/LmeSzinc/AzurLaneAutoScript/tree/master/module/device) 是对python代码的C++翻译)

2.2.1 模拟器截图方式: nemu_ipc, ascreencap, adb, scrcpy(有损)   

对应代码 include/device/screenshot

TODO: adb_nc, droidcast, ascreencap_nc

2.2.2 模拟器控制方式: adb, scrcpy, nemu 

对应代码 include/device/control

TODO: maatouch, minitouch, hermit

2.3 特征识别系统:

简介:在我的理解下, 脚本的本质在于**检测某个特征(feature)进行对应操作(action)**, 所以BAAS框架在设计时就着重考虑了特征的多样性问题

include/feature 你可以继承BaseFeature类来定义任意形式的特征(无论是参数还是比较方式), 你可以使用opencv, ocr, 甚至更多的AI模型来判断图像中出现了哪些特征

注: 参数的任意性通过json实现, 如果你不了解json请先了解2.1中提及的仓库

ps:文档不全面, 请等待文档完善后使用这个框架
