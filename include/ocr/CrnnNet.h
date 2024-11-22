#ifndef __OCR_CRNNNET_H__
#define __OCR_CRNNNET_H__

#include "OcrStruct.h"
#include "onnxruntime/onnxruntime_cxx_api.h"
#include "opencv2/opencv.hpp"

class CrnnNet {
public:
    static CrnnNet* get_net(const std::string& model_path, const std::string& keys_path);

    static bool release_net(const std::string& model_path, const std::string& keys_path);

    static inline std::string model_key_joined_path(const std::string& model_path, const std::string& keys_path);

    static void release_all();

    ~CrnnNet();

    void setNumThread(int numOfThread);

    void setGpuIndex(int gpuIndex);

    void initModel();

    std::vector<TextLine> getTextLines(std::vector<cv::Mat> &partImg, const char *path, const char *imgName);

    TextLine getTextLine(const cv::Mat &src);

private:
    void initModel(const std::string &pathStr, const std::string &keysPath);

    std::string modelPath, keyDictPath;

    static std::map <std::string, CrnnNet*> nets;

    bool isOutputDebugImg = false;
    Ort::Session *session;
    Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "CrnnNet");
    Ort::SessionOptions sessionOptions = Ort::SessionOptions();
    int numThread = 0;

    std::vector<Ort::AllocatedStringPtr> inputNamesPtr;
    std::vector<Ort::AllocatedStringPtr> outputNamesPtr;

    const float meanValues[3] = {127.5, 127.5, 127.5};
    const float normValues[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};
    const int dstHeight = 48;

    std::vector<std::string> keys;

    TextLine scoreToTextLine(const std::vector<float> &outputData, size_t h, size_t w);

};


#endif //__OCR_CRNNNET_H__
