#include "ocr/CrnnNet.h"
#include "ocr/OcrUtils.h"
#include <fstream>
#include <numeric>
#include "BAASGlobals.h"
#include "config/BAASGlobalSetting.h"

#ifdef __DIRECTML__
#include <onnxruntime/core/providers/dml/dml_provider_factory.h>
#endif

BAAS_NAMESPACE_BEGIN

void CrnnNet::setGpuIndex(int gpuIndex)
{
#ifdef __CUDA__
    if (gpuIndex >= 0) {
        OrtCUDAProviderOptions cuda_options;
        cuda_options.device_id = gpuIndex;
        cuda_options.arena_extend_strategy = 0;
        cuda_options.gpu_mem_limit = 2ULL * 1024 * 1024 * 1024;
        cuda_options.cudnn_conv_algo_search = OrtCudnnConvAlgoSearchDefault;
        cuda_options.do_copy_in_default_stream = 1;

        sessionOptions.AppendExecutionProvider_CUDA(cuda_options);
        printf("rec try to use GPU%d\n", gpuIndex);
    } else {
        printf("rec use CPU\n");
    }
#endif

#ifdef __DIRECTML__
    if (gpuIndex >= 0) {
        OrtSessionOptionsAppendExecutionProvider_DML(sessionOptions, gpuIndex);
        printf("rec try to use GPU%d\n", gpuIndex);
    }
    else {
        printf("rec use CPU\n");
    }
#endif
}

std::map<std::string, CrnnNet *> CrnnNet::nets;

CrnnNet::~CrnnNet()
{
    delete session;
    inputNamesPtr.clear();
    outputNamesPtr.clear();
}

void CrnnNet::setNumThread(int numOfThread)
{
    numThread = numOfThread;
    //===session options===
    // Sets the number of threads used to parallelize the execution within nodes
    // A value of 0 means ORT will pick a default
    //sessionOptions.SetIntraOpNumThreads(numThread);
    //set OMP_NUM_THREADS=16

    // Sets the number of threads used to parallelize the execution of the graph (across nodes)
    // If sequential execution is enabled this value is ignored
    // A value of 0 means ORT will pick a default
    sessionOptions.SetInterOpNumThreads(numThread);
    sessionOptions.SetIntraOpNumThreads(numThread);

    // Sets graph optimization level
    // ORT_DISABLE_ALL -> To disable all optimizations
    // ORT_ENABLE_BASIC -> To enable basic optimizations (Such as redundant node removals)
    // ORT_ENABLE_EXTENDED -> To enable extended optimizations (Includes level 1 + more complex optimizations like node fusions)
    // ORT_ENABLE_ALL -> To Enable All possible opitmizations
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
}

void CrnnNet::initModel(
        const std::string &pathStr,
        const std::string &keysPath
)
{
#ifdef _WIN32
    std::wstring crnnPath = strToWstr(pathStr);
    session = new Ort::Session(env, crnnPath.c_str(), sessionOptions);
#else
    session = new Ort::Session(env, pathStr.c_str(), sessionOptions);
#endif
    modelPath = pathStr;
    keyDictPath = keysPath;
    inputNamesPtr = getInputNames(session);
    outputNamesPtr = getOutputNames(session);

    //load keys
    std::ifstream in(keysPath.c_str());
    std::string line;
    keys.clear();
    keys.emplace_back("#");
    if (in) {
        while (getline(in, line)) {// line中不包括每行的换行符
            keys.push_back(line);
            if (character2Index.find(line) != character2Index.end()) {
                BAASGlobalLogger->BAASError("keys.txt has duplicate keys");
                return;
            }
            character2Index[line] = keys.size() - 1;
        }
    } else {
        BAASGlobalLogger->BAASError("keys.txt not found");
        return;
    }
    keys.emplace_back(" ");
    BAASGlobalLogger->BAASInfo("keys size : " + std::to_string(keys.size()));
}

template<class ForwardIterator>
inline static size_t argmax(
        ForwardIterator first,
        ForwardIterator last
)
{
    return std::distance(first, std::max_element(first, last));
}


TextLine CrnnNet::scoreToTextLine(
        const std::vector<float> &outputData,
        size_t h,
        size_t w
)
{
    /*
     *  w : size of key dict
     */
    auto keySize = keys.size();
    auto dataSize = outputData.size();
    std::string strRes;
    std::vector<float> scores;
    size_t lastIndex = 0;
    size_t maxIndex;
    float maxValue;

    for (size_t i = 0; i < h; i++) {
        size_t start = i * w;
        size_t stop = (i + 1) * w;
        if (stop > dataSize - 1) {
            stop = (i + 1) * w - 1;
        }
        maxIndex = int(argmax(&outputData[start], &outputData[stop]));
        maxValue = float(*std::max_element(&outputData[start], &outputData[stop]));

        if (maxIndex > 0 && maxIndex < keySize &&
            (!(i > 0 && maxIndex == lastIndex))) {        // every letter is divided by " "
            scores.emplace_back(maxValue);
            strRes.append(keys[maxIndex]);
        }
        lastIndex = maxIndex;
    }
    return {strRes, scores};
}

TextLine CrnnNet::scoreToTextLine(
        const std::vector<float> &outputData,
        size_t h,
        size_t w,
        const std::vector<size_t> &enabledIndexes
)
{
    /*
    *  w : size of key dict
    */
    auto keySize = keys.size();
    size_t dataSize;
    std::string strRes;
    std::vector<float> scores;
    size_t lastIndex = 0;
    size_t maxIndex;
    float maxValue;

    std::vector<float> enabledScores;
    size_t i, j;
    for (i = 0; i < h; i++)
        for (j = 0; j <= enabledIndexes.size() - 1; j++)
            enabledScores.push_back(outputData[i * w + enabledIndexes[j]]);

    w = enabledIndexes.size();
    dataSize = enabledScores.size();
    for (i = 0; i < h; i++) {
        size_t start = i * w;
        size_t stop = (i + 1) * w;
        if (stop > dataSize - 1) {
            stop = (i + 1) * w - 1;
        }
        maxIndex = int(argmax(&enabledScores[start], &enabledScores[stop]));
        maxValue = float(*std::max_element(&enabledScores[start], &enabledScores[stop]));

        if (maxIndex > 0 && maxIndex < keySize &&
            (!(i > 0 && maxIndex == lastIndex))) {        // every letter is divided by "#"
            scores.emplace_back(maxValue);
            strRes.append(keys[enabledIndexes[maxIndex]]);
        }
        lastIndex = maxIndex;
    }
    return {strRes, scores};
}

TextLine CrnnNet::getTextLine(const cv::Mat &src)
{
    float scale = (float) dstHeight / (float) src.rows;
    int dstWidth = int((float) src.cols * scale);
    cv::Mat srcResize;
    resize(src, srcResize, cv::Size(dstWidth, dstHeight));
    std::vector<float> inputTensorValues = substractMeanNormalize(srcResize, meanValues, normValues);
    std::array<int64_t, 4> inputShape{1, srcResize.channels(), srcResize.rows, srcResize.cols};
    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, inputTensorValues.data(),
            inputTensorValues.size(), inputShape.data(),
            inputShape.size());
    assert(inputTensor.IsTensor());
    std::vector<const char *> inputNames = {inputNamesPtr.data()
                                                         ->get()};
    std::vector<const char *> outputNames = {outputNamesPtr.data()
                                                           ->get()};
    auto outputTensor = session->Run(
            Ort::RunOptions{nullptr}, inputNames.data(), &inputTensor,
            inputNames.size(), outputNames.data(), outputNames.size());
    assert(outputTensor.size() == 1 && outputTensor.front()
                                                   .IsTensor());
    std::vector<int64_t> outputShape = outputTensor[0].GetTensorTypeAndShapeInfo()
                                                      .GetShape();
    int64_t outputCount = std::accumulate(
            outputShape.begin(), outputShape.end(), 1,
            std::multiplies<int64_t>());
    float *floatArray = outputTensor.front()
                                    .GetTensorMutableData<float>();
    std::vector<float> outputData(floatArray, floatArray + outputCount);
    return scoreToTextLine(outputData, outputShape[1], outputShape[2]);
}

TextLine CrnnNet::getTextLine(
        const cv::Mat &src,
        const std::vector<size_t> &enabledIndexes
)
{
    float scale = (float) dstHeight / (float) src.rows;
    int dstWidth = int((float) src.cols * scale);
    cv::Mat srcResize;
    resize(src, srcResize, cv::Size(dstWidth, dstHeight));
    std::vector<float> inputTensorValues = substractMeanNormalize(srcResize, meanValues, normValues);
    std::array<int64_t, 4> inputShape{1, srcResize.channels(), srcResize.rows, srcResize.cols};
    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, inputTensorValues.data(),
            inputTensorValues.size(), inputShape.data(),
            inputShape.size());
    assert(inputTensor.IsTensor());
    std::vector<const char *> inputNames = {inputNamesPtr.data()
                                                         ->get()};
    std::vector<const char *> outputNames = {outputNamesPtr.data()
                                                           ->get()};
    auto outputTensor = session->Run(
            Ort::RunOptions{nullptr}, inputNames.data(), &inputTensor,
            inputNames.size(), outputNames.data(), outputNames.size());
    assert(outputTensor.size() == 1 && outputTensor.front()
                                                   .IsTensor());
    std::vector<int64_t> outputShape = outputTensor[0].GetTensorTypeAndShapeInfo()
                                                      .GetShape();
    int64_t outputCount = std::accumulate(
            outputShape.begin(), outputShape.end(), 1,
            std::multiplies<int64_t>());
    float *floatArray = outputTensor.front()
                                    .GetTensorMutableData<float>();
    std::vector<float> outputData(floatArray, floatArray + outputCount);
    return scoreToTextLine(outputData, outputShape[1], outputShape[2], enabledIndexes);
}


std::vector<TextLine> CrnnNet::getTextLines(
        std::vector<cv::Mat> &partImg,
        const char *path,
        const char *imgName
)
{
    int size = int(partImg.size());
    std::vector<TextLine> textLines(size);
    std::vector<size_t> enabledIndexes;
    for (int i = 0; i < size; ++i) {
        //OutPut DebugImg
        if (isOutputDebugImg) {
            std::string debugImgFile = getDebugImgFilePath(path, imgName, i, "-debug-");
            saveImg(partImg[i], debugImgFile.c_str());
        }

        //getTextLine
        cv::flip(partImg[i], partImg[i], 1);
        cv::flip(partImg[i], partImg[i], 1);
        double startCrnnTime = getCurrentTime();
        TextLine textLine = getTextLine(partImg[i]);
        double endCrnnTime = getCurrentTime();
        textLine.time = endCrnnTime - startCrnnTime;
        textLines[i] = textLine;
    }
    return textLines;
}

std::vector<TextLine> CrnnNet::getTextLines(
        std::vector<cv::Mat> &partImg,
        const char *path,
        const char *imgName,
        const std::vector<std::string> &candidates
)
{
    int size = int(partImg.size());
    std::vector<TextLine> textLines(size);
    std::vector<size_t> enabledIndexes;
    getTextIndexes(candidates, enabledIndexes);
    for (int i = 0; i < size; ++i) {
        //OutPut DebugImg
        if (isOutputDebugImg) {
            std::string debugImgFile = getDebugImgFilePath(path, imgName, i, "-debug-");
            saveImg(partImg[i], debugImgFile.c_str());
        }

        //getTextLine
        cv::flip(partImg[i], partImg[i], 1);
        cv::flip(partImg[i], partImg[i], 1);
        double startCrnnTime = getCurrentTime();
        TextLine textLine = getTextLine(partImg[i], enabledIndexes);
        double endCrnnTime = getCurrentTime();
        textLine.time = endCrnnTime - startCrnnTime;
        textLines[i] = textLine;
    }
    return textLines;
}

CrnnNet *CrnnNet::get_net(
        const std::string &model_path,
        const std::string &keys_path
)
{
    std::string joined_path = model_key_joined_path(model_path, keys_path);
    auto it = nets.find(joined_path);
    if (it != nets.end()) return it->second;

    auto *net = new CrnnNet();

    net->modelPath = BAAS_OCR_MODEL_DIR + "\\" + model_path;
    net->keyDictPath = BAAS_OCR_MODEL_DIR + "\\" + keys_path;
    net->setGpuIndex(global_setting->ocr_gpu_id());
    net->setNumThread(global_setting->ocr_num_thread());
    nets[joined_path] = net;
    return net;
}

bool CrnnNet::release_net(
        const std::string &model_path,
        const std::string &keys_path
)
{
    std::string joined_path = model_key_joined_path(model_path, keys_path);
    auto it = nets.find(joined_path);
    if (it == nets.end()) return false;
    delete it->second;
    nets.erase(it);
    return true;
}

void CrnnNet::release_all()
{
    for (auto &it: nets) delete it.second;
    nets.clear();
}

std::string CrnnNet::model_key_joined_path(
        const std::string &model_path,
        const std::string &keys_path
)
{
    return model_path + " | " + keys_path;
}

void CrnnNet::initModel()
{
    initModel(modelPath, keyDictPath);
}

void CrnnNet::getTextIndexes(
        const std::vector<std::string> &characters,
        std::vector<size_t> &enabledIndexes
)
{
    enabledIndexes.clear();
    enabledIndexes.push_back(0);
    for (size_t i = 0; i < characters.size(); i++) {
        auto it = character2Index.find(characters[i]);
        if (it != character2Index.end()) enabledIndexes.push_back(it->second);
    }
}


BAAS_NAMESPACE_END

