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

void OcrLite::get_text_boxes(
        const cv::Mat &img,
        std::vector<TextBox> &result
)
{
    cv::Mat originSrc = img;
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
    result = dbNet->getTextBoxes(paddingSrc, scale, boxScoreThresh, boxThresh, unClipRatio);
    for (auto &textBox: result) {
        for (auto &point: textBox.boxPoint) {
            point.x = (std::min)((std::max)(int(point.x - padding), 0), originSrc.cols - 1);
            point.y = (std::min)((std::max)(int(point.y - padding), 0), originSrc.rows - 1);
        }
    }
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
                    crnnNet.get(),
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
                    crnnNet.get(),
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
                dbNet.get(),
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
                angleNet.get(),
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
                    crnnNet.get(),
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
                    crnnNet.get(),
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
        boxPoint[0] = cv::Point(
                std::max(textBoxes[i].boxPoint[0].x - padding, 0),
                std::max(textBoxes[i].boxPoint[0].y - padding, 0)
                );
        boxPoint[1] = cv::Point(
                std::min(textBoxes[i].boxPoint[1].x - padding, src.cols - 1),
                std::max(textBoxes[i].boxPoint[1].y - padding, 0)
                );
        boxPoint[2] = cv::Point(
                std::min(textBoxes[i].boxPoint[2].x - padding, src.cols - 1),
                std::min(textBoxes[i].boxPoint[2].y - padding, src.rows - 1)
                );
        boxPoint[3] = cv::Point(
                std::max(textBoxes[i].boxPoint[3].x - padding, 0),
                std::min(textBoxes[i].boxPoint[3].y - padding, src.rows - 1)
                );
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
    cv::Mat textBoxImg;
    std::string strRes;
    for (auto &textBlock: textBlocks) {
        if (textBlock.text.empty())
            continue;
        strRes.append(textBlock.text);
        strRes.append("\n");
    }

    double fullTime = endTime - startTime;
    return OcrResult{dbNetTime, textBlocks, fullTime, strRes};
}

void OcrLite::get_net(
        const std::filesystem::path &detPath,
        const std::filesystem::path &clsPath,
        const std::filesystem::path &recPath,
        const std::filesystem::path &keysPath,
        int gpu_id,
        int num_thread,
        bool enable_cpu_memory_arena
)
{
    dbNet    = DbNet::get_net(detPath, gpu_id, num_thread, enable_cpu_memory_arena);
    angleNet = AngleNet::get_net(clsPath, gpu_id, num_thread, enable_cpu_memory_arena);
    crnnNet  = CrnnNet::get_net(recPath, keysPath, gpu_id, num_thread, enable_cpu_memory_arena);

    BAASGlobalLogger->BAASInfo("DBNet    : " + detPath.string());   
    BAASGlobalLogger->BAASInfo("Use count: " + std::to_string(dbNet.use_count()));
    BAASGlobalLogger->BAASInfo("AngleNet : " + clsPath.string());
    BAASGlobalLogger->BAASInfo("Use count: " + std::to_string(angleNet.use_count()));
    BAASGlobalLogger->BAASInfo("CrnnNet  : " + recPath.string());
    BAASGlobalLogger->BAASInfo("Use count: " + std::to_string(crnnNet.use_count()));
}

BAAS_NAMESPACE_END
