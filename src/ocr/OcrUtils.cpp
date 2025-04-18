#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <numeric>
#include "ocr/OcrUtils.h"
#include "ocr/clipper.h"

BAAS_NAMESPACE_BEGIN


void from_json(
        const nlohmann::json &j,
        cv::Point &point
)
{
    point.x = j.at(0).get<int>();
    point.y = j.at(1).get<int>();
}

void to_json(
    nlohmann::json& j, 
    const cv::Point& p
) 
{
    j = nlohmann::json::array({p.x, p.y});
}

void to_json(
        nlohmann::json& j,
        const TextBox& box
)
{
    assert(box.boxPoint.size() == 4);

    j = nlohmann::json::array();
    nlohmann::json temp;
    to_json(temp, box.boxPoint[0]);
    j.push_back(temp);
    to_json(temp, box.boxPoint[1]);
    j.push_back(temp);
    to_json(temp, box.boxPoint[2]);
    j.push_back(temp);
    to_json(temp, box.boxPoint[3]);
    j.push_back(temp);
    j.push_back(box.score);
}

void from_json(
        const nlohmann::json &j,
        TextBox &box
)
{
    assert (j.is_array());
    assert (j.size() == 5);
    box.boxPoint.resize(4);
    from_json(j.at(0), box.boxPoint[0]);
    from_json(j.at(1), box.boxPoint[1]);
    from_json(j.at(2), box.boxPoint[2]);
    from_json(j.at(3), box.boxPoint[3]);
    box.score = j.at(4).get<float>();
}


double getCurrentTime()
{
    return (static_cast<double>(cv::getTickCount())) / cv::getTickFrequency() * 1000;//单位毫秒
}

ScaleParam getScaleParam(
        cv::Mat &src,
        const int targetSize
)
{
    int srcWidth, srcHeight, dstWidth, dstHeight;
    srcWidth = dstWidth = src.cols;
    srcHeight = dstHeight = src.rows;

    float ratio = 1.f;
    if (srcWidth > srcHeight) {
        ratio = float(targetSize) / float(srcWidth);
    } else {
        ratio = float(targetSize) / float(srcHeight);
    }
    dstWidth = int(float(srcWidth) * ratio);
    dstHeight = int(float(srcHeight) * ratio);
    if (dstWidth % 32 != 0) {
        dstWidth = (dstWidth / 32) * 32;
        dstWidth = (std::max)(dstWidth, 32);
    }
    if (dstHeight % 32 != 0) {
        dstHeight = (dstHeight / 32) * 32;
        dstHeight = (std::max)(dstHeight, 32);
    }
    float ratioWidth = (float) dstWidth / (float) srcWidth;
    float ratioHeight = (float) dstHeight / (float) srcHeight;
    return {srcWidth, srcHeight, dstWidth, dstHeight, ratioWidth, ratioHeight};
}

std::vector<cv::Point2f> getBox(const cv::RotatedRect &rect)
{
    cv::Point2f vertices[4];
    rect.points(vertices);
    //std::vector<cv::Point2f> ret(4);
    std::vector<cv::Point2f> ret2(vertices, vertices + sizeof(vertices) / sizeof(vertices[0]));
    //memcpy(vertices, &ret[0], ret.size() * sizeof(ret[0]));
    return ret2;
}


cv::Mat matRotateClockWise180(cv::Mat src)
{
    flip(src, src, 0);
    flip(src, src, 1);
    return src;
}

cv::Mat matRotateClockWise90(cv::Mat src)
{
    transpose(src, src);
    flip(src, src, 1);
    return src;
}

cv::Mat getRotateCropImage(
        const cv::Mat &src,
        std::vector<cv::Point> box
)
{
    cv::Mat image;
    src.copyTo(image);
    std::vector<cv::Point> points = box;

    int collectX[4] = {box[0].x, box[1].x, box[2].x, box[3].x};
    int collectY[4] = {box[0].y, box[1].y, box[2].y, box[3].y};
    int left = int(*std::min_element(collectX, collectX + 4));
    int right = int(*std::max_element(collectX, collectX + 4));
    int top = int(*std::min_element(collectY, collectY + 4));
    int bottom = int(*std::max_element(collectY, collectY + 4));

    cv::Mat imgCrop;
    image(cv::Rect(left, top, right - left, bottom - top)).copyTo(imgCrop);

    for (auto &point: points) {
        point.x -= left;
        point.y -= top;
    }

    int imgCropWidth = int(
            sqrt(
                    pow(points[0].x - points[1].x, 2) +
                    pow(points[0].y - points[1].y, 2)));
    int imgCropHeight = int(
            sqrt(
                    pow(points[0].x - points[3].x, 2) +
                    pow(points[0].y - points[3].y, 2)));

    cv::Point2f ptsDst[4];
    ptsDst[0] = cv::Point2f(0., 0.);
    ptsDst[1] = cv::Point2f(imgCropWidth, 0.);
    ptsDst[2] = cv::Point2f(imgCropWidth, imgCropHeight);
    ptsDst[3] = cv::Point2f(0.f, imgCropHeight);

    cv::Point2f ptsSrc[4];
    ptsSrc[0] = cv::Point2f(points[0].x, points[0].y);
    ptsSrc[1] = cv::Point2f(points[1].x, points[1].y);
    ptsSrc[2] = cv::Point2f(points[2].x, points[2].y);
    ptsSrc[3] = cv::Point2f(points[3].x, points[3].y);

    cv::Mat M = cv::getPerspectiveTransform(ptsSrc, ptsDst);

    cv::Mat partImg;
    cv::warpPerspective(
            imgCrop, partImg, M,
            cv::Size(imgCropWidth, imgCropHeight),
            cv::BORDER_REPLICATE
    );

    if (float(partImg.rows) >= float(partImg.cols) * 1.5) {
        cv::Mat srcCopy = cv::Mat(partImg.rows, partImg.cols, partImg.depth());
        cv::transpose(partImg, srcCopy);
        cv::flip(srcCopy, srcCopy, 0);
        return srcCopy;
    } else {
        return partImg;
    }
}

cv::Mat adjustTargetImg(
        cv::Mat &src,
        int dstWidth,
        int dstHeight
)
{
    cv::Mat srcResize;
    float scale = (float) dstHeight / (float) src.rows;
    int angleWidth = int((float) src.cols * scale);
    cv::resize(src, srcResize, cv::Size(angleWidth, dstHeight));
    cv::Mat srcFit = cv::Mat(dstHeight, dstWidth, CV_8UC3, cv::Scalar(255, 255, 255));
    if (angleWidth < dstWidth) {
        cv::Rect rect(0, 0, srcResize.cols, srcResize.rows);
        srcResize.copyTo(srcFit(rect));
    } else {
        cv::Rect rect(0, 0, dstWidth, dstHeight);
        srcResize(rect).copyTo(srcFit);
    }
    return srcFit;
}

bool cvPointCompare(
        const cv::Point &a,
        const cv::Point &b
)
{
    return a.x < b.x;
}

std::vector<cv::Point2f> getMinBoxes(
        const cv::RotatedRect &boxRect,
        float &maxSideLen
)
{
    maxSideLen = std::max(
            boxRect.size
                   .width, boxRect.size
                                  .height
    );
    std::vector<cv::Point2f> boxPoint = getBox(boxRect);
    std::sort(boxPoint.begin(), boxPoint.end(), cvPointCompare);
    int index1, index2, index3, index4;
    if (boxPoint[1].y > boxPoint[0].y) {
        index1 = 0;
        index4 = 1;
    } else {
        index1 = 1;
        index4 = 0;
    }
    if (boxPoint[3].y > boxPoint[2].y) {
        index2 = 2;
        index3 = 3;
    } else {
        index2 = 3;
        index3 = 2;
    }
    std::vector<cv::Point2f> minBox(4);
    minBox[0] = boxPoint[index1];
    minBox[1] = boxPoint[index2];
    minBox[2] = boxPoint[index3];
    minBox[3] = boxPoint[index4];
    return minBox;
}

float boxScoreFast(
        const std::vector<cv::Point2f> &boxes,
        const cv::Mat &pred
)
{
    int width = pred.cols;
    int height = pred.rows;

    float arrayX[4] = {boxes[0].x, boxes[1].x, boxes[2].x, boxes[3].x};
    float arrayY[4] = {boxes[0].y, boxes[1].y, boxes[2].y, boxes[3].y};

    int minX = clamp(int(std::floor(*(std::min_element(arrayX, arrayX + 4)))), 0, width - 1);
    int maxX = clamp(int(std::ceil(*(std::max_element(arrayX, arrayX + 4)))), 0, width - 1);
    int minY = clamp(int(std::floor(*(std::min_element(arrayY, arrayY + 4)))), 0, height - 1);
    int maxY = clamp(int(std::ceil(*(std::max_element(arrayY, arrayY + 4)))), 0, height - 1);

    cv::Mat mask = cv::Mat::zeros(maxY - minY + 1, maxX - minX + 1, CV_8UC1);

    cv::Point box[4];
    box[0] = cv::Point(int(boxes[0].x) - minX, int(boxes[0].y) - minY);
    box[1] = cv::Point(int(boxes[1].x) - minX, int(boxes[1].y) - minY);
    box[2] = cv::Point(int(boxes[2].x) - minX, int(boxes[2].y) - minY);
    box[3] = cv::Point(int(boxes[3].x) - minX, int(boxes[3].y) - minY);
    const cv::Point *pts[1] = {box};
    int npts[] = {4};
    cv::fillPoly(mask, pts, npts, 1, cv::Scalar(1));

    cv::Mat croppedImg;
    pred(cv::Rect(minX, minY, maxX - minX + 1, maxY - minY + 1))
            .copyTo(croppedImg);

    auto score = (float) cv::mean(croppedImg, mask)[0];
    return score;
}

float getContourArea(
        const std::vector<cv::Point2f> &box,
        float unClipRatio
)
{
    size_t size = box.size();
    float area = 0.0f;
    float dist = 0.0f;
    for (size_t i = 0; i < size; i++) {
        area += box[i].x * box[(i + 1) % size].y -
                box[i].y * box[(i + 1) % size].x;
        dist += sqrtf((box[i].x - box[(i + 1) % size].x) *
                      (box[i].x - box[(i + 1) % size].x) +
                      (box[i].y - box[(i + 1) % size].y) *
                      (box[i].y - box[(i + 1) % size].y));
    }
    area = fabs(float(area / 2.0));

    return area * unClipRatio / dist;
}

cv::RotatedRect unClip(
        std::vector<cv::Point2f> box,
        float unClipRatio
)
{
    float distance = getContourArea(box, unClipRatio);

    ClipperLib::ClipperOffset offset;
    ClipperLib::Path p;
    p << ClipperLib::IntPoint(int(box[0].x), int(box[0].y))
      << ClipperLib::IntPoint(int(box[1].x), int(box[1].y))
      << ClipperLib::IntPoint(int(box[2].x), int(box[2].y))
      << ClipperLib::IntPoint(int(box[3].x), int(box[3].y));
    offset.AddPath(p, ClipperLib::jtRound, ClipperLib::etClosedPolygon);

    ClipperLib::Paths soln;
    offset.Execute(soln, distance);
    std::vector<cv::Point2f> points;

    for (size_t j = 0; j < soln.size(); j++) {
        for (size_t i = 0; i < soln[soln.size() - 1].size(); i++) {
            points.emplace_back(soln[j][i].X, soln[j][i].Y);
        }
    }
    cv::RotatedRect res;
    if (points.empty()) {
        res = cv::RotatedRect(cv::Point2f(0, 0), cv::Size2f(1, 1), 0);
    } else {
        res = cv::minAreaRect(points);
    }
    return res;
}

std::vector<float> substractMeanNormalize(
        cv::Mat &src,
        const float *meanVals,
        const float *normVals
)
{
    auto inputTensorSize = src.cols * src.rows * src.channels();
    std::vector<float> inputTensorValues(inputTensorSize);
    size_t numChannels = src.channels();
    size_t imageSize = src.cols * src.rows;

    for (size_t pid = 0; pid < imageSize; pid++) {
        for (size_t ch = 0; ch < numChannels; ++ch) {
            float data = (float) (src.data[pid * numChannels + ch] * normVals[ch] - meanVals[ch] * normVals[ch]);
            inputTensorValues[ch * imageSize + pid] = data;
        }
    }
    return inputTensorValues;
}

std::vector<int> getAngleIndexes(std::vector<Angle> &angles)
{
    std::vector<int> angleIndexes;
    angleIndexes.reserve(angles.size());
    for (auto &angle: angles) {
        angleIndexes.push_back(angle.index);
    }
    return angleIndexes;
}

std::vector<Ort::AllocatedStringPtr> getInputNames(const std::unique_ptr<Ort::Session>& session)
{
    Ort::AllocatorWithDefaultOptions allocator;
    const size_t numInputNodes = session->GetInputCount();

    std::vector<Ort::AllocatedStringPtr> inputNamesPtr;
    inputNamesPtr.reserve(numInputNodes);
    std::vector<int64_t> input_node_dims;

    // iterate over all input nodes
    for (size_t i = 0; i < numInputNodes; i++) {
        auto inputName = session->GetInputNameAllocated(i, allocator);
        inputNamesPtr.push_back(std::move(inputName));
        /*printf("inputName[%zu] = %s\n", i, inputName.get());

        // print input node types
        auto typeInfo = session->GetInputTypeInfo(i);
        auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();

        ONNXTensorElementDataType type = tensorInfo.GetElementType();
        printf("inputType[%zu] = %u\n", i, type);

        // print input shapes/dims
        input_node_dims = tensorInfo.GetShape();
        printf("Input num_dims = %zu\n", input_node_dims.size());
        for (size_t j = 0; j < input_node_dims.size(); j++) {
            printf("Input dim[%zu] = %llu\n",j, input_node_dims[j]);
        }*/
    }
    return inputNamesPtr;
}

std::vector<Ort::AllocatedStringPtr> getOutputNames(const std::unique_ptr<Ort::Session>& session)
{
    Ort::AllocatorWithDefaultOptions allocator;
    const size_t numOutputNodes = session->GetOutputCount();

    std::vector<Ort::AllocatedStringPtr> outputNamesPtr;
    outputNamesPtr.reserve(numOutputNodes);
    std::vector<int64_t> output_node_dims;

    for (size_t i = 0; i < numOutputNodes; i++) {
        auto outputName = session->GetOutputNameAllocated(i, allocator);
        outputNamesPtr.push_back(std::move(outputName));
        /*printf("outputName[%zu] = %s\n", i, outputName.get());

        // print input node types
        auto type_info = session->GetOutputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

        ONNXTensorElementDataType type = tensor_info.GetElementType();
        printf("outputType[%zu] = %u\n", i, type);

        // print input shapes/dims
        output_node_dims = tensor_info.GetShape();
        printf("output num_dims = %zu\n", output_node_dims.size());
        for (size_t j = 0; j < output_node_dims.size(); j++) {
            printf("output dim[%zu] = %llu\n",j, output_node_dims[j]);
        }*/
    }
    return outputNamesPtr;
}

BAAS_NAMESPACE_END