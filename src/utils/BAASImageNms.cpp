#include "utils/BAASImageUtil.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>

BAAS_NAMESPACE_BEGIN

namespace {

inline double rect_area(const cv::Rect &rect)
{
    return static_cast<double>(rect.width) * static_cast<double>(rect.height);
}

inline float rect_iou(const cv::Rect &a, const cv::Rect &b)
{
    const int left = std::max(a.x, b.x);
    const int top = std::max(a.y, b.y);
    const int right = std::min(a.x + a.width, b.x + b.width);
    const int bottom = std::min(a.y + a.height, b.y + b.height);

    const int intersection_width = std::max(0, right - left);
    const int intersection_height = std::max(0, bottom - top);
    const double intersection_area =
            static_cast<double>(intersection_width) * static_cast<double>(intersection_height);
    if (intersection_area <= 0.0) return 0.0f;

    const double union_area = rect_area(a) + rect_area(b) - intersection_area;
    if (union_area <= 0.0) return 0.0f;
    return static_cast<float>(intersection_area / union_area);
}

} // namespace

void BAASImageUtil::nms_boxes(
        const std::vector<cv::Rect> &boxes,
        const std::vector<float> &scores,
        float score_threshold,
        float nms_threshold,
        std::vector<int> &indices,
        float eta,
        int top_k
)
{
    indices.clear();
    if (boxes.size() != scores.size()) {
        throw std::invalid_argument("BAASImageUtil::nms_boxes requires boxes.size() == scores.size()");
    }
    if (score_threshold < 0.0f) {
        throw std::invalid_argument("BAASImageUtil::nms_boxes requires score_threshold >= 0");
    }
    if (nms_threshold < 0.0f) {
        throw std::invalid_argument("BAASImageUtil::nms_boxes requires nms_threshold >= 0");
    }
    if (eta <= 0.0f) {
        throw std::invalid_argument("BAASImageUtil::nms_boxes requires eta > 0");
    }
    if (top_k < 0) {
        throw std::invalid_argument("BAASImageUtil::nms_boxes requires top_k >= 0");
    }

    std::vector<std::pair<float, int>> candidates;
    candidates.reserve(scores.size());
    for (int i = 0; i < static_cast<int>(scores.size()); ++i) {
        if (scores[i] > score_threshold) {
            candidates.emplace_back(scores[i], i);
        }
    }

    std::stable_sort(
            candidates.begin(),
            candidates.end(),
            [](const std::pair<float, int> &left, const std::pair<float, int> &right) {
                return left.first > right.first;
            }
    );

    if (top_k > 0 && top_k < static_cast<int>(candidates.size())) {
        candidates.resize(static_cast<std::size_t>(top_k));
    }

    indices.reserve(candidates.size());
    float adaptive_threshold = nms_threshold;
    for (const auto &candidate : candidates) {
        const int candidate_index = candidate.second;
        bool keep = true;
        for (const int kept_index : indices) {
            if (rect_iou(boxes[candidate_index], boxes[kept_index]) > adaptive_threshold) {
                keep = false;
                break;
            }
        }
        if (keep) {
            indices.push_back(candidate_index);
            if (eta < 1.0f && adaptive_threshold > 0.5f) {
                adaptive_threshold *= eta;
            }
        }
    }
}

BAAS_NAMESPACE_END
