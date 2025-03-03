#include <string>

#include "ocr/OcrLite.h"
#include "ocr/OcrUtils.h"
#include "ocr/BAASOCR.h"
using namespace cv;
using namespace std;

BAAS_NAMESPACE_BEGIN

OcrLite::OcrLite() {}

OcrLite::~OcrLite()
{

}


cv::Mat makePadding(
        cv::Mat &src,
        const int padding
)
{
    if (padding <= 0) return src;
    cv::Scalar paddingScalar = {255, 255, 255};
    cv::Mat paddingSrc;
    cv::copyMakeBorder(src, paddingSrc, padding, padding, padding, padding, cv::BORDER_ISOLATED, paddingScalar);
    return paddingSrc;
}

OcrResult OcrLite::detectImageBytes(
        const uint8_t *data,
        const long dataLength,
        const int grey,
        const int padding,
        const int maxSideLen,
        float boxScoreThresh,
        float boxThresh,
        float unClipRatio,
        bool doAngle,
        bool mostAngle
)
{
    std::vector<uint8_t> vecData(data, data + dataLength);
    cv::Mat originSrc = cv::imdecode(vecData, grey == 1 ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR);//default : BGR
    OcrResult result;
    result = detect(
            originSrc, padding, maxSideLen,
            boxScoreThresh, boxThresh, unClipRatio, doAngle, mostAngle
    );
    return result;

}

OcrResult OcrLite::detectBitmap(
        uint8_t *bitmapData,
        int width,
        int height,
        int channels,
        int padding,
        int maxSideLen,
        float boxScoreThresh,
        float boxThresh,
        float unClipRatio,
        bool doAngle,
        bool mostAngle
)
{

    auto *originSrc = new cv::Mat(height, width, CV_8UC(channels), bitmapData);
    if (channels > 3) {
        cv::cvtColor(*originSrc, *originSrc, cv::COLOR_RGBA2BGR);
    } else if (channels == 3) {
        cv::cvtColor(*originSrc, *originSrc, cv::COLOR_RGB2BGR);
    }
    OcrResult result;
    result = detect(
            *originSrc, padding, maxSideLen,
            boxScoreThresh, boxThresh, unClipRatio, doAngle, mostAngle
    );
    return result;
}


OcrResult OcrLite::detect(
        const cv::Mat &mat,
        int padding,
        int maxSideLen,
        float boxScoreThresh,
        float boxThresh,
        float unClipRatio,
        bool doAngle,
        bool mostAngle,
        const std::vector<std::string> &candidates
)
{
    cv::Mat originSrc = mat;
    int originMaxSide = (std::max)(originSrc.cols, originSrc.rows);
    int resize;
    if (maxSideLen <= 0 || maxSideLen > originMaxSide) {
        resize = originMaxSide;
    } else {
        resize = maxSideLen;
    }
    resize += 2 * padding;
    cv::Rect paddingRect(padding, padding, originSrc.cols, originSrc.rows);
    cv::Mat paddingSrc = makePadding(originSrc, padding);
    ScaleParam scale = getScaleParam(paddingSrc, resize);
    OcrResult result;
    result = detect(
            paddingSrc,
            paddingRect,
            scale,
            boxScoreThresh,
            boxThresh,
            unClipRatio,
            doAngle,
            mostAngle,
            candidates
    );
    return result;
}

void OcrLite::ocr_for_single_line(
        const cv::Mat &img,
        TextLine &text,
        const vector<string> &candidates
)
{
    double startCrnnTime = getCurrentTime();
    if (candidates.empty()) {
        if (BAASOCR::thread_pool_enabled) {
            text = BAASOCR::pool->submit(
                    submit_getTextLine,
                    crnnNet,
                    img
            ).get();
        }
        else {
            text = crnnNet->getTextLine(img);
        }
    }
    else {
        std::vector<size_t> enabledIndexes;
        crnnNet->getTextIndexes(candidates, enabledIndexes);
        if (BAASOCR::thread_pool_enabled) {
            text = BAASOCR::pool->submit(
                    submit_getTextLine_withEnabledIndexes,
                    crnnNet,
                    img,
                    enabledIndexes
            ).get();
        }
        else {
            text = crnnNet->getTextLine(img, enabledIndexes);
        }
    }
    text.time = getCurrentTime() - startCrnnTime;

}

std::vector<cv::Mat> OcrLite::getPartImages(
        cv::Mat &src,
        std::vector<TextBox> &textBoxes
)
{
    std::vector<cv::Mat> partImages;
    for (size_t i = 0; i < textBoxes.size(); ++i) {
        cv::Mat partImg = getRotateCropImage(src, textBoxes[i].boxPoint);
        partImages.emplace_back(partImg);
    }
    return partImages;
}

OcrResult OcrLite::detect(
        cv::Mat &src,
        cv::Rect &originRect,
        ScaleParam &scale,
        float boxScoreThresh,
        float boxThresh,
        float unClipRatio,
        bool doAngle,
        bool mostAngle,
        const std::vector<std::string> &candidates
)
{
    //   dbNet getTextBoxes
    double startTime = getCurrentTime();
    std::vector<TextBox> textBoxes;
    if (BAASOCR::thread_pool_enabled) {
        textBoxes = BAASOCR::pool->submit(
                submit_getTextBoxes,
                dbNet,
                src,
                scale,
                boxScoreThresh,
                boxThresh,
                unClipRatio
        ).get();
    }
    else {
        textBoxes = dbNet->getTextBoxes(src, scale, boxScoreThresh, boxThresh, unClipRatio);
    }


    double endDbNetTime = getCurrentTime();
    double dbNetTime = endDbNetTime - startTime;

    //---------- getPartImages ----------
    std::vector<cv::Mat> partImages = getPartImages(src, textBoxes);

    // AngleNet get angles
    std::vector<Angle> angles;
    if (BAASOCR::thread_pool_enabled) {
        angles = BAASOCR::pool->submit(
                submit_getAngles,
                angleNet,
                partImages,
                doAngle,
                mostAngle
        ).get();
    }
    else {
        angles = angleNet->getAngles(partImages, doAngle, mostAngle);
    }

    //Rotate partImgs
    for (size_t i = 0; i < partImages.size(); ++i) {
        if (angles[i].index == 1) {
            partImages.at(i) = matRotateClockWise180(partImages[i]);
        }
    }

    // CrnnNet get textlines
    std::vector<TextLine> textLines;

    if (!candidates.empty()) {
        if (BAASOCR::thread_pool_enabled) {
            textLines = BAASOCR::pool->submit(
                    submit_getTextLines_withCandidates,
                    crnnNet,
                    partImages,
                    candidates
            ).get();
        }
        else {
            textLines = crnnNet->getTextLines(partImages, candidates);
        }
    }
    else {
        if (BAASOCR::thread_pool_enabled) {
            textLines = BAASOCR::pool->submit(
                    submit_getTextLines,
                    crnnNet,
                    partImages
            ).get();
        }
        else {
            textLines = crnnNet->getTextLines(partImages);
        }
    }

    std::vector<TextBlock> textBlocks;
    for (size_t i = 0; i < textLines.size(); ++i) {
        std::vector<cv::Point> boxPoint = std::vector<cv::Point>(4);
        int padding = originRect.x;//padding conversion
        boxPoint[0] = cv::Point(textBoxes[i].boxPoint[0].x - padding, textBoxes[i].boxPoint[0].y - padding);
        boxPoint[1] = cv::Point(textBoxes[i].boxPoint[1].x - padding, textBoxes[i].boxPoint[1].y - padding);
        boxPoint[2] = cv::Point(textBoxes[i].boxPoint[2].x - padding, textBoxes[i].boxPoint[2].y - padding);
        boxPoint[3] = cv::Point(textBoxes[i].boxPoint[3].x - padding, textBoxes[i].boxPoint[3].y - padding);
        TextBlock textBlock{
            boxPoint,
            textBoxes[i].score,
            angles[i].index,
            angles[i].score,
            angles[i].time,
            textLines[i].text,
            textLines[i].charScores,
            textLines[i].time
        };
        textBlocks.emplace_back(textBlock);
    }

    double endTime = getCurrentTime();
    double fullTime = endTime - startTime;

    cv::Mat textBoxImg;
    std::string strRes;
    for (auto &textBlock: textBlocks) {
        if (textBlock.text.empty())
            continue;
        strRes.append(textBlock.text);
        strRes.append("\n");
    }

    return OcrResult{dbNetTime, textBlocks, fullTime, strRes};
}

void OcrLite::get_net(
        const std::filesystem::path &detPath,
        const std::filesystem::path &clsPath,
        const std::filesystem::path &recPath,
        const std::filesystem::path &keysPath,
        int gpu_id,
        int num_thread
)
{
    dbNet = DbNet::get_net(detPath, gpu_id, num_thread);
    angleNet = AngleNet::get_net(clsPath, gpu_id, num_thread);
    crnnNet = CrnnNet::get_net(recPath, keysPath, gpu_id, num_thread);

    std::cout << dbNet->getModelPath() << std::endl;
    std::cout<< dbNet.use_count() << std::endl;

    std::cout << angleNet->getModelPath() << std::endl;
    std::cout<< angleNet.use_count() << std::endl;

    std::cout << crnnNet->get_joined_path() << std::endl;
    std::cout<< crnnNet.use_count() << std::endl;
}

BAAS_NAMESPACE_END
