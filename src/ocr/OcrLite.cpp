#include "ocr/OcrLite.h"
#include "ocr/OcrUtils.h"
#include <string>

using namespace cv;
using namespace std;

BAAS_NAMESPACE_BEGIN

OcrLite::OcrLite() {}

OcrLite::~OcrLite()
{
    if (isOutputResultTxt) {
        fclose(resultTxt);
    }
}

void OcrLite::set_num_thread(int num_thread)
{
    dbNet->set_num_thread(num_thread);
    angleNet->set_num_thread(num_thread);
    crnnNet->set_num_thread(num_thread);
}

void OcrLite::initLogger(
        bool isConsole,
        bool isPartImg,
        bool isResultImg
)
{
    isOutputConsole = isConsole;
    isOutputPartImg = isPartImg;
    isOutputResultImg = isResultImg;
}

void OcrLite::enableResultTxt(
        const char *path,
        const char *imgName
)
{
    isOutputResultTxt = true;
    std::string resultTxtPath = getResultTxtFilePath(path, imgName);
    printf("resultTxtPath(%s)\n", resultTxtPath.c_str());
    resultTxt = fopen(resultTxtPath.c_str(), "w");
}

void OcrLite::set_gpu_id(int gpu_id)
{
    dbNet->set_gpu_id(gpu_id);
    angleNet->set_gpu_id(gpu_id);
    crnnNet->set_gpu_id(gpu_id);
}

bool OcrLite::initModels()
{
    dbNet->initModel();
    angleNet->initModel();
    crnnNet->initModel();
    return true;
}

void OcrLite::Logger(
        const char *format,
        ...
)
{
    if (!(isOutputConsole || isOutputResultTxt)) return;
    char *buffer = (char *) malloc(8192);
    va_list args;
            va_start(args, format);
    vsprintf(buffer, format, args);
            va_end(args);
    if (isOutputConsole) printf("%s", buffer);
    if (isOutputResultTxt) fprintf(resultTxt, "%s", buffer);

    free(buffer);
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

OcrResult OcrLite::detect(
        const char *path,
        const char *imgName,
        const int padding,
        const int maxSideLen,
        float boxScoreThresh,
        float boxThresh,
        float unClipRatio,
        bool doAngle,
        bool mostAngle,
        const std::vector<std::string> &candidates
)
{
    std::string imgFile = getSrcImgFilePath(path, imgName);
    cv::Mat originSrc = imread(imgFile, cv::IMREAD_COLOR);//default : BGR
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
            path, imgName, paddingSrc, paddingRect, scale,
            boxScoreThresh, boxThresh, unClipRatio, doAngle, mostAngle, candidates
    );
    return result;
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
            NULL, NULL, paddingSrc, paddingRect, scale,
            boxScoreThresh, boxThresh, unClipRatio, doAngle, mostAngle, candidates
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
    if (candidates.empty()) text = crnnNet->getTextLine(img);
    else {
        std::vector<size_t> enabledIndexes;
        crnnNet->getTextIndexes(candidates, enabledIndexes);
        text = crnnNet->getTextLine(img, enabledIndexes);
    }
    text.time = getCurrentTime() - startCrnnTime;

}

std::vector<cv::Mat> OcrLite::getPartImages(
        cv::Mat &src,
        std::vector<TextBox> &textBoxes,
        const char *path,
        const char *imgName
)
{
    std::vector<cv::Mat> partImages;
    for (size_t i = 0; i < textBoxes.size(); ++i) {
        cv::Mat partImg = getRotateCropImage(src, textBoxes[i].boxPoint);
        partImages.emplace_back(partImg);
        //OutPut DebugImg
        if (isOutputPartImg) {
            std::string debugImgFile = getDebugImgFilePath(path, imgName, i, "-part-");
            saveImg(partImg, debugImgFile.c_str());
        }
    }
    return partImages;
}

OcrResult OcrLite::detect(
        const char *path,
        const char *imgName,
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
    cv::Mat textBoxPaddingImg = src.clone();
    int thickness = getThickness(src);

    Logger("=====Start detect=====\n");
    Logger(
            "ScaleParam(sw:%d,sh:%d,dw:%d,dh:%d,%f,%f)\n", scale.srcWidth, scale.srcHeight,
            scale.dstWidth, scale.dstHeight,
            scale.ratioWidth, scale.ratioHeight
    );

    Logger("---------- step: dbNet getTextBoxes ----------\n");
    double startTime = getCurrentTime();
    std::vector<TextBox> textBoxes = dbNet->getTextBoxes(src, scale, boxScoreThresh, boxThresh, unClipRatio);
    double endDbNetTime = getCurrentTime();
    double dbNetTime = endDbNetTime - startTime;
    Logger("dbNetTime(%fms)\n", dbNetTime);

    for (size_t i = 0; i < textBoxes.size(); ++i) {
        Logger(
                "TextBox[%d][score(%f),[x: %d, y: %d], [x: %d, y: %d], [x: %d, y: %d], [x: %d, y: %d]]\n", i,
                textBoxes[i].score,
                textBoxes[i].boxPoint[0].x, textBoxes[i].boxPoint[0].y,
                textBoxes[i].boxPoint[1].x, textBoxes[i].boxPoint[1].y,
                textBoxes[i].boxPoint[2].x, textBoxes[i].boxPoint[2].y,
                textBoxes[i].boxPoint[3].x, textBoxes[i].boxPoint[3].y
        );
    }

    Logger("---------- step: drawTextBoxes ----------\n");
    drawTextBoxes(textBoxPaddingImg, textBoxes, thickness);

    //---------- getPartImages ----------
    std::vector<cv::Mat> partImages = getPartImages(src, textBoxes, path, imgName);

    Logger("---------- step: angleNet getAngles ----------\n");
    std::vector<Angle> angles;
    angles = angleNet->getAngles(partImages, path, imgName, doAngle, mostAngle);

    //Log Angles
    for (size_t i = 0; i < angles.size(); ++i) {
        Logger("angle[%d][index(%d), score(%f), time(%fms)]\n", i, angles[i].index, angles[i].score, angles[i].time);
    }

    //Rotate partImgs
    for (size_t i = 0; i < partImages.size(); ++i) {
        if (angles[i].index == 1) {
            partImages.at(i) = matRotateClockWise180(partImages[i]);
        }
    }

    Logger("---------- step: crnnNet getTextLine ----------\n");
    std::vector<TextLine> textLines;

    if (!candidates.empty()) textLines = crnnNet->getTextLines(partImages, path, imgName, candidates);
    else textLines = crnnNet->getTextLines(partImages, path, imgName);

    //Log TextLines
    for (size_t i = 0; i < textLines.size(); ++i) {
        Logger(
                "textLine[%d](%s)\n", i, textLines[i].text
                                                     .c_str());
        std::ostringstream txtScores;
        for (size_t s = 0; s < textLines[i].charScores
                                           .size(); ++s) {
            if (s == 0) {
                txtScores << textLines[i].charScores[s];
            } else {
                txtScores << " ," << textLines[i].charScores[s];
            }
        }
        Logger("textScores[%d]{%s}\n", i, std::string(txtScores.str()).c_str());
        Logger("crnnTime[%d](%fms)\n", i, textLines[i].time);
    }

    std::vector<TextBlock> textBlocks;
    for (size_t i = 0; i < textLines.size(); ++i) {
        std::vector<cv::Point> boxPoint = std::vector<cv::Point>(4);
        int padding = originRect.x;//padding conversion
        boxPoint[0] = cv::Point(textBoxes[i].boxPoint[0].x - padding, textBoxes[i].boxPoint[0].y - padding);
        boxPoint[1] = cv::Point(textBoxes[i].boxPoint[1].x - padding, textBoxes[i].boxPoint[1].y - padding);
        boxPoint[2] = cv::Point(textBoxes[i].boxPoint[2].x - padding, textBoxes[i].boxPoint[2].y - padding);
        boxPoint[3] = cv::Point(textBoxes[i].boxPoint[3].x - padding, textBoxes[i].boxPoint[3].y - padding);
        TextBlock textBlock{boxPoint, textBoxes[i].score, angles[i].index, angles[i].score,
                            angles[i].time, textLines[i].text, textLines[i].charScores, textLines[i].time};
        textBlocks.emplace_back(textBlock);
    }

    double endTime = getCurrentTime();
    double fullTime = endTime - startTime;
    Logger("=====End detect=====\n");
    Logger("FullDetectTime(%fms)\n", fullTime);

    //cropped to original size
    cv::Mat textBoxImg;

    if (originRect.x > 0 && originRect.y > 0) {
        textBoxPaddingImg(originRect).copyTo(textBoxImg);
    } else {
        textBoxImg = textBoxPaddingImg;
    }

    //Save result.jpg
    if (isOutputResultImg) {
        std::string resultImgFile = getResultImgFilePath(path, imgName);
        imwrite(resultImgFile, textBoxImg);
    }

    std::string strRes;
    for (auto &textBlock: textBlocks) {
        if (textBlock.text
                     .empty())
            continue;
        strRes.append(textBlock.text);
        strRes.append("\n");
    }

    return OcrResult{dbNetTime, textBlocks, textBoxImg, fullTime, strRes};
}

void OcrLite::get_net(
        const std::filesystem::path &detPath,
        const std::filesystem::path &clsPath,
        const std::filesystem::path &recPath,
        const std::filesystem::path &keysPath
)
{
    dbNet = DbNet::get_net(detPath);
    angleNet = AngleNet::get_net(clsPath);
    crnnNet = CrnnNet::get_net(recPath, keysPath);
}

BAAS_NAMESPACE_END
