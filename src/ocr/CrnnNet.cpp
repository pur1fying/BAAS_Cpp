#include <fstream>
#include <memory>
#include <numeric>

#include "ocr/CrnnNet.h"
#include "ocr/OcrUtils.h"
#include "BAASGlobals.h"
#include "config/BAASGlobalSetting.h"
#include "ocr/BAASOCR.h"

#ifdef __DIRECTML__
#include <onnxruntime/core/providers/dml/dml_provider_factory.h>
#endif

BAAS_NAMESPACE_BEGIN

void CrnnNet::set_gpu_id(int gpu_id)
{
#ifdef __CUDA__
    if (gpu_id >= 0) {
        OrtCUDAProviderOptions cuda_options;
        cuda_options.device_id = gpu_id;
        cuda_options.arena_extend_strategy = 0;
        cuda_options.gpu_mem_limit = 2ULL * 1024 * 1024 * 1024;
        cuda_options.cudnn_conv_algo_search = OrtCudnnConvAlgoSearchDefault;
        cuda_options.do_copy_in_default_stream = 1;

        sessionOptions.AppendExecutionProvider_CUDA(cuda_options);
        BAASGlobalLogger->BAASInfo("Rec try to use GPU " + std::to_string(gpu_id));
    } else {
        BAASGlobalLogger->BAASInfo("Rec use CPU.");
    }
#else
    if (gpu_id >= 0) {
        BAASGlobalLogger->BAASWarn("Rec not support GPU.");
    }
    BAASGlobalLogger->BAASInfo("Rec use CPU.");
#endif

#ifdef __DIRECTML__
    if (gpu_id >= 0) {
        OrtSessionOptionsAppendExecutionProvider_DML(sessionOptions, gpu_id);
        printf("rec try to use GPU%d\n", gpu_id);
    }
    else {
        printf("rec use CPU\n");
    }
#endif
}

std::map<std::string, std::shared_ptr<CrnnNet>> CrnnNet::nets;

CrnnNet::~CrnnNet()
{
    session.reset();
    inputNamesPtr.clear();
    outputNamesPtr.clear();
}

void CrnnNet::set_num_thread(int num_thread)
{
    numThread = num_thread;
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
        const std::filesystem::path &path,
        const std::filesystem::path &keysPath
)
{
#ifdef _WIN32
    std::wstring crnnPath = path.wstring();
    session = std::make_unique<Ort::Session>(env, crnnPath.c_str(), sessionOptions);
#else
    session = std::make_unique<Ort::Session>(env, path.c_str(), sessionOptions);
#endif
    modelPath = path;
    keyDictPath = keysPath;
    inputNamesPtr = getInputNames(session);
    outputNamesPtr = getOutputNames(session);

    //load keys
    std::ifstream in(keysPath.c_str());
    std::string line;
    keys.clear();
    keys.emplace_back("#");
    if (in) {
        while (getline(in, line)) {
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
    in.close();
    keys.emplace_back(" ");
    character2Index[" "] = keys.size() - 1;
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

        if (maxIndex > 0 && maxIndex < keySize && (!(i > 0 && maxIndex == lastIndex))) {        // every letter is divided by " "
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

        if (maxIndex > 0 && maxIndex < keySize && (!(i > 0 && maxIndex == lastIndex))) {        // every letter is divided by "#"
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
    std::vector<const char *> inputNames = {inputNamesPtr.data()->get()};
    std::vector<const char *> outputNames = {outputNamesPtr.data()->get()};
    auto outputTensor = session->Run(
            Ort::RunOptions{nullptr}, inputNames.data(), &inputTensor,
            inputNames.size(), outputNames.data(), outputNames.size());
    assert(outputTensor.size() == 1 && outputTensor.front()
                                                   .IsTensor());
    std::vector<int64_t> outputShape = outputTensor[0].GetTensorTypeAndShapeInfo()
                                                      .GetShape();
    int64_t outputCount = std::accumulate(outputShape.begin(), outputShape.end(), 1, std::multiplies<int64_t>());
    float *floatArray = outputTensor.front().GetTensorMutableData<float>();
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
    std::vector<const char *> inputNames = {inputNamesPtr.data()->get()};
    std::vector<const char *> outputNames = {outputNamesPtr.data()->get()};
    auto outputTensor = session->Run(
            Ort::RunOptions{nullptr}, inputNames.data(), &inputTensor,
            inputNames.size(), outputNames.data(), outputNames.size());
    assert(outputTensor.size() == 1 && outputTensor.front()
                                                   .IsTensor());
    std::vector<int64_t> outputShape = outputTensor[0].GetTensorTypeAndShapeInfo()
                                                      .GetShape();
    int64_t outputCount = std::accumulate(outputShape.begin(), outputShape.end(), 1,std::multiplies<int64_t>());
    float *floatArray = outputTensor.front()
                                    .GetTensorMutableData<float>();
    std::vector<float> outputData(floatArray, floatArray + outputCount);
    return scoreToTextLine(outputData, outputShape[1], outputShape[2], enabledIndexes);
}


std::vector<TextLine> CrnnNet::getTextLines(
        std::vector<cv::Mat> &partImg
)
{
    int size = int(partImg.size());
    std::vector<TextLine> textLines(size);
    std::vector<size_t> enabledIndexes;
    for (int i = 0; i < size; ++i) {

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
        const std::vector<std::string> &candidates
)
{
    int size = int(partImg.size());
    std::vector<TextLine> textLines(size);
    std::vector<size_t> enabledIndexes;
    getTextIndexes(candidates, enabledIndexes);
    for (int i = 0; i < size; ++i) {
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

std::shared_ptr<CrnnNet> CrnnNet::get_net(
        const std::filesystem::path &model_path,
        const std::filesystem::path &keys_path,
        int gpu_id,
        int num_thread
)
{
    std::string joined_path = model_key_joined_path(
            BAAS_OCR_MODEL_DIR / model_path,
            BAAS_OCR_MODEL_DIR / keys_path
            );
    auto it = nets.find(joined_path);
    if (it != nets.end()) {
        BAASGlobalLogger->BAASInfo("Rec Already Inited");
        return it->second;
    }

    auto net = std::make_shared<CrnnNet>();
    net->modelPath = BAAS_OCR_MODEL_DIR / model_path;
    net->keyDictPath = BAAS_OCR_MODEL_DIR / keys_path;
    net->set_gpu_id(gpu_id);
    net->set_num_thread(num_thread);
//    net->initModel();

    nets[joined_path] = net;
    BAASOCR::uninited_crnnnet.push_back(net.get());
    return net;
}

bool CrnnNet::try_release_net(
        const std::string& joined_path
)
{
    auto it = nets.find(joined_path);
    if (it != nets.end()) {
        auto count = it->second.use_count();
        if (count > 1) {
            BAASGlobalLogger->BAASWarn("Rec : " + it->first + " use_count : " + std::to_string(count) + " > 1, can't release.");
            return false;
        }
        it->second.reset();
        BAASGlobalLogger->BAASInfo("Rec Release : " + it->first);
        nets.erase(it);
        return true;
    }
    return true;
}

void CrnnNet::try_release_all()
{
    BAASGlobalLogger->sub_title("Rec Release All");
    for (auto it = nets.begin(); it != nets.end();) {
        auto count = it->second.use_count();
        if (count > 1) {
            BAASGlobalLogger->BAASWarn("Rec : " + it->first + " use_count : " + std::to_string(count) + " > 1, can't release.");
            ++it;
            continue;
        }
        it->second.reset();
        BAASGlobalLogger->BAASInfo("Rec Release : " + it->first);
        it = nets.erase(it);
    }
    BAASGlobalLogger->BAASInfo("CrnnNet map size : " + std::to_string(baas::CrnnNet::nets.size()));

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

