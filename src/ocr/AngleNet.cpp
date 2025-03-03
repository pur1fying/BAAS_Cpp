#include <numeric>

#include "ocr/AngleNet.h"
#include "ocr/OcrUtils.h"
#include "BAASGlobals.h"
#include "ocr/BAASOCR.h"

#ifdef __DIRECTML__
#include <onnxruntime/core/providers/dml/dml_provider_factory.h>
#endif

BAAS_NAMESPACE_BEGIN

std::map<std::string, std::shared_ptr<AngleNet>> AngleNet::nets;

void AngleNet::set_gpu_id(int gpu_id)
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
        BAASGlobalLogger->BAASInfo("Cls try to use GPU " + std::to_string(gpu_id));
    } else {
        BAASGlobalLogger->BAASInfo("Cls use CPU.");
    }
#else
    if (gpu_id >= 0) {
        BAASGlobalLogger->BAASWarn("Cls not support GPU.");
    }
    BAASGlobalLogger->BAASInfo("Cls use CPU.");
#endif

#ifdef __DIRECTML__
    if (gpu_id >= 0) {
        OrtSessionOptionsAppendExecutionProvider_DML(sessionOptions, gpu_id);
        printf("cls try to use GPU%d\n", gpu_id);
    }
    else {
        printf("cls use CPU\n");
    }
#endif
}

AngleNet::~AngleNet()
{
    session.reset();
    inputNamesPtr.clear();
    outputNamesPtr.clear();
}

void AngleNet::set_num_thread(int num_thread)
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

void AngleNet::initModel(const std::filesystem::path &path)
{

#ifdef _WIN32
    std::wstring anglePath = path.wstring();
    session = std::make_unique<Ort::Session>(env, anglePath.c_str(), sessionOptions);
#else
    session = std::make_unique<Ort::Session>(env, path.c_str(), sessionOptions);
#endif
    inputNamesPtr = getInputNames(session);
    outputNamesPtr = getOutputNames(session);
}

Angle scoreToAngle(const std::vector<float> &outputData)
{
    int maxIndex = 0;
    float maxScore = 0;
    for (size_t i = 0; i < outputData.size(); i++) {
        if (outputData[i] > maxScore) {
            maxScore = outputData[i];
            maxIndex = int(i);
        }
    }
    return {maxIndex, maxScore};
}

Angle AngleNet::getAngle(cv::Mat &src)
{
    std::vector<float> inputTensorValues = substractMeanNormalize(src, meanValues, normValues);
    std::array<int64_t, 4> inputShape{1, src.channels(), src.rows, src.cols};
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
    int64_t outputCount = std::accumulate(
            outputShape.begin(), outputShape.end(), 1,
            std::multiplies<int64_t>());
    float *floatArray = outputTensor.front()
                                    .GetTensorMutableData<float>();
    std::vector<float> outputData(floatArray, floatArray + outputCount);
    return scoreToAngle(outputData);
}

std::vector<Angle> AngleNet::getAngles(
        std::vector<cv::Mat> &partImgs,
        bool doAngle,
        bool mostAngle
)
{
    size_t size = partImgs.size();
    std::vector<Angle> angles(size);
    if (doAngle) {
        for (size_t i = 0; i < size; ++i) {
            double startAngle = getCurrentTime();
            cv::Mat angleImg;
            cv::resize(partImgs[i], angleImg, cv::Size(dstWidth, dstHeight));
            Angle angle = getAngle(angleImg);
            double endAngle = getCurrentTime();
            angle.time = endAngle - startAngle;
            angles[i] = angle;
        }
    } else {
        for (size_t i = 0; i < size; ++i) {
            angles[i] = Angle{-1, 0.f};
        }
    }
    //Most Possible AngleIndex
    if (doAngle && mostAngle) {
        auto angleIndexes = getAngleIndexes(angles);
        double sum = std::accumulate(angleIndexes.begin(), angleIndexes.end(), 0.0);
        double halfPercent = angles.size() / 2.0f;
        int mostAngleIndex;
        if (sum < halfPercent) {//all angle set to 0
            mostAngleIndex = 0;
        } else {//all angle set to 1
            mostAngleIndex = 1;
        }
        //printf("Set All Angle to mostAngleIndex(%d)\n", mostAngleIndex);
        for (size_t i = 0; i < angles.size(); ++i) {
            Angle angle = angles[i];
            angle.index = mostAngleIndex;
            angles.at(i) = angle;
        }
    }

    return angles;
}

std::shared_ptr<AngleNet> AngleNet::get_net(
        const std::filesystem::path &model_path,
        int gpu_id,
        int num_thread
)
{
    auto it = nets.find((BAAS_OCR_MODEL_DIR / model_path).string());
    if (it != nets.end()) {
        BAASGlobalLogger->BAASInfo("Cls Already Inited");
        return it->second;
    }
    auto net = std::make_shared<AngleNet>();
    net->modelPath = BAAS_OCR_MODEL_DIR / model_path;
    net->set_gpu_id(gpu_id);
    net->set_num_thread(num_thread);
//    net->initModel();

    nets[net->modelPath.string()] = net;
    BAASOCR::uninited_anglenet.push_back(net.get());
    return net;
}

bool AngleNet::try_release_net(const std::string &model_path)
{
    auto it = nets.find(model_path);
    if (it != nets.end()) {
        auto count = it->second.use_count();
        if (count > 1) {
            BAASGlobalLogger->BAASWarn("Cls : " + it->first + " use_count : " + std::to_string(count) + " > 1, can't release.");
            return false;
        }
        it->second.reset();
        BAASGlobalLogger->BAASInfo("Cls Release : " + it->first);
        nets.erase(it);
        return true;
    }
    return false;
}

void AngleNet::initModel()
{
    initModel(modelPath);
}

void AngleNet::try_release_all()
{
    BAASGlobalLogger->sub_title("Cls Release All");
    for (auto it = nets.begin(); it != nets.end();) {
        auto count = it->second.use_count();
        if (count > 1) {
            BAASGlobalLogger->BAASWarn("Cls : " + it->first + " use_count : " + std::to_string(count) + " > 1, can't release.");
            ++it;
            continue;
        }
        it->second.reset();
        BAASGlobalLogger->BAASInfo("Cls Release : " + it->first);
        it = nets.erase(it);
    }
    BAASGlobalLogger->BAASInfo("Angle map size : " + std::to_string(baas::AngleNet::nets.size()));
}

BAAS_NAMESPACE_END

