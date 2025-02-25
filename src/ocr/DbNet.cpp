#include "ocr/DbNet.h"
#include "ocr/OcrUtils.h"
#include "BAASGlobals.h"
#include "config/BAASGlobalSetting.h"

BAAS_NAMESPACE_BEGIN

std::map<std::string, DbNet *> DbNet::nets;

void DbNet::set_gpu_id(int gpu_id)
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
        BAASGlobalLogger->BAASInfo("Det try to use GPU " + std::to_string(gpu_id));
    } else {
        BAASGlobalLogger->BAASInfo("Det use CPU");
    }
#else
    if (gpu_id >= 0) {
        BAASGlobalLogger->BAASWarn("Det not support GPU");
    }
    BAASGlobalLogger->BAASInfo("Det use CPU");
#endif
}

DbNet::~DbNet()
{
    delete session;
    inputNamesPtr.clear();
    outputNamesPtr.clear();
}

void DbNet::set_num_thread(int num_thread)
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

void DbNet::initModel(const std::filesystem::path &pathStr)
{
    session = new Ort::Session(env, pathStr.c_str(), sessionOptions);
    modelPath = pathStr;
    inputNamesPtr = getInputNames(session);
    outputNamesPtr = getOutputNames(session);
}

std::vector<TextBox> findRsBoxes(
        const cv::Mat &predMat,
        const cv::Mat &dilateMat,
        ScaleParam &s,
        const float boxScoreThresh,
        const float unClipRatio
)
{
    const int longSideThresh = 3;//minBox 长边门限
    const int maxCandidates = 1000;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(
            dilateMat, contours, hierarchy, cv::RETR_LIST,
            cv::CHAIN_APPROX_SIMPLE
    );

    size_t numContours = contours.size() >= maxCandidates ? maxCandidates : contours.size();

    std::vector<TextBox> rsBoxes;

    for (size_t i = 0; i < numContours; i++) {
        if (contours[i].size() <= 2) {
            continue;
        }
        cv::RotatedRect minAreaRect = cv::minAreaRect(contours[i]);

        float longSide;
        std::vector<cv::Point2f> minBoxes = getMinBoxes(minAreaRect, longSide);

        if (longSide < longSideThresh) {
            continue;
        }

        float boxScore = boxScoreFast(minBoxes, predMat);
        if (boxScore < boxScoreThresh)
            continue;

        //-----unClip-----
        cv::RotatedRect clipRect = unClip(minBoxes, unClipRatio);
        if (clipRect.size
                    .height < 1.001 && clipRect.size
                                               .width < 1.001) {
            continue;
        }
        //-----unClip-----

        std::vector<cv::Point2f> clipMinBoxes = getMinBoxes(clipRect, longSide);
        if (longSide < longSideThresh + 2)
            continue;

        std::vector<cv::Point> intClipMinBoxes;

        for (auto &clipMinBox: clipMinBoxes) {
            float x = clipMinBox.x / s.ratioWidth;
            float y = clipMinBox.y / s.ratioHeight;
            int ptX = (std::min)((std::max)(int(x), 0), s.srcWidth - 1);
            int ptY = (std::min)((std::max)(int(y), 0), s.srcHeight - 1);
            cv::Point point{ptX, ptY};
            intClipMinBoxes.push_back(point);
        }
        rsBoxes.push_back(TextBox{intClipMinBoxes, boxScore});
    }
    reverse(rsBoxes.begin(), rsBoxes.end());
    return rsBoxes;
}

std::vector<TextBox>
DbNet::getTextBoxes(
        cv::Mat &src,
        ScaleParam &s,
        float boxScoreThresh,
        float boxThresh,
        float unClipRatio
)
{
    cv::Mat srcResize;
    resize(src, srcResize, cv::Size(s.dstWidth, s.dstHeight));
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

    //-----Data preparation-----
    int outHeight = (int) outputShape[2];
    int outWidth = (int) outputShape[3];
    size_t area = outHeight * outWidth;

    std::vector<float> predData(area, 0.0);
    std::vector<unsigned char> cbufData(area, ' ');

    for (int i = 0; i < area; i++) {
        predData[i] = float(outputData[i]);
        cbufData[i] = (unsigned char) ((outputData[i]) * 255);
    }

    cv::Mat predMat(outHeight, outWidth, CV_32F, (float *) predData.data());
    cv::Mat cBufMat(outHeight, outWidth, CV_8UC1, (unsigned char *) cbufData.data());

    //-----boxThresh-----
    const double maxValue = 255;
    const double threshold = boxThresh * 255;
    cv::Mat thresholdMat;
    cv::threshold(cBufMat, thresholdMat, threshold, maxValue, cv::THRESH_BINARY);

    //-----dilate-----
    cv::Mat dilateMat;
    cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::dilate(thresholdMat, dilateMat, dilateElement);

    return findRsBoxes(predMat, dilateMat, s, boxScoreThresh, unClipRatio);
}

DbNet* DbNet::get_net(const std::filesystem::path &model_path)
{
    auto it = nets.find(model_path.string());
    if (it != nets.end()) {
        BAASGlobalLogger->BAASInfo("Det Already Inited");
        return it->second;
    }
    auto *net = new DbNet();
    net->modelPath = BAAS_OCR_MODEL_DIR / model_path;
    net->set_gpu_id(global_setting->ocr_gpu_id());
    net->set_num_thread(global_setting->ocr_num_thread());
    nets[model_path.string()] = net;
    return net;
}

void DbNet::initModel()
{
    initModel(modelPath);
}


bool DbNet::release_net(const std::filesystem::path &model_path)
{
    auto it = nets.find(model_path.string());
    if (it != nets.end()) {
        delete it->second;
        nets.erase(it);
        return true;
    }
    return false;
}

void DbNet::release_all()
{
    for (auto &it: nets) delete it.second;
    nets.clear();
}


BAAS_NAMESPACE_END
