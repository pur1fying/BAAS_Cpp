#include <cstddef>
#include <cstdlib>

#include <exception>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>
#include <opencv2/dnn.hpp>

#include "utils/BAASImageUtil.h"

namespace {

struct NmsDataset {
    std::vector<cv::Rect> boxes;
    std::vector<float> scores;
};

struct NmsCase {
    int count;
    float score_threshold;
    float nms_threshold;
    float eta;
    int top_k;
};

NmsDataset make_dataset(int count, int seed_offset = 0)
{
    std::mt19937 box_rng(12345 + seed_offset);
    std::mt19937 score_rng(67890 + seed_offset);
    std::uniform_int_distribution<int> xy(-80, 720);
    std::uniform_int_distribution<int> size(4, 160);
    std::uniform_real_distribution<float> score(0.0f, 1.0f);

    NmsDataset dataset;
    dataset.boxes.reserve(static_cast<std::size_t>(count));
    dataset.scores.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        dataset.boxes.emplace_back(xy(box_rng), xy(box_rng), size(box_rng), size(box_rng));
        dataset.scores.push_back(score(score_rng));
    }
    return dataset;
}

const NmsDataset& dataset_for_count(int count)
{
    static const NmsDataset dataset_100 = make_dataset(100);
    static const NmsDataset dataset_1000 = make_dataset(1000);
    static const NmsDataset dataset_8400 = make_dataset(8400);

    switch (count) {
        case 100:
            return dataset_100;
        case 1000:
            return dataset_1000;
        case 8400:
            return dataset_8400;
        default:
            std::abort();
    }
}

void require_indices_equal(
        const std::vector<int>& baas_indices,
        const std::vector<int>& opencv_indices,
        const std::string& case_name
)
{
    if (baas_indices == opencv_indices) return;

    std::cerr << case_name << " mismatch\nBAAS:";
    for (int index : baas_indices) std::cerr << ' ' << index;
    std::cerr << "\nOpenCV:";
    for (int index : opencv_indices) std::cerr << ' ' << index;
    std::cerr << '\n';
    throw std::runtime_error(case_name + " mismatch");
}

void compare_case(
        const std::vector<cv::Rect>& boxes,
        const std::vector<float>& scores,
        float score_threshold,
        float nms_threshold,
        float eta,
        int top_k,
        const std::string& case_name
)
{
    std::vector<int> baas_indices = {42, 43};
    std::vector<int> opencv_indices = {42, 43};
    baas::BAASImageUtil::nms_boxes(boxes, scores, score_threshold, nms_threshold, baas_indices, eta, top_k);
    cv::dnn::NMSBoxes(boxes, scores, score_threshold, nms_threshold, opencv_indices, eta, top_k);
    require_indices_equal(baas_indices, opencv_indices, case_name);
}

void run_explicit_correctness_cases()
{
    compare_case({}, {}, 0.1f, 0.5f, 1.0f, 0, "empty input");

    compare_case(
            {cv::Rect(0, 0, 10, 10), cv::Rect(20, 20, 10, 10)},
            {0.5f, 0.5001f},
            0.5f,
            0.5f,
            1.0f,
            0,
            "strict score threshold"
    );

    compare_case(
            {cv::Rect(0, 0, 10, 10), cv::Rect(20, 20, 10, 10), cv::Rect(40, 40, 10, 10)},
            {0.5f, 0.5f, 0.5f},
            0.1f,
            0.5f,
            1.0f,
            0,
            "stable equal scores"
    );

    compare_case(
            {cv::Rect(0, 0, 10, 10), cv::Rect(0, 0, 10, 10), cv::Rect(0, 0, 10, 10)},
            {0.7f, 0.9f, 0.8f},
            0.1f,
            0.5f,
            1.0f,
            0,
            "fully overlapping boxes"
    );

    compare_case(
            {cv::Rect(0, 0, 10, 10), cv::Rect(5, 0, 10, 10), cv::Rect(2, 0, 10, 10)},
            {0.9f, 0.8f, 0.7f},
            0.1f,
            0.3334f,
            1.0f,
            0,
            "partial overlap boundary"
    );

    compare_case(
            {cv::Rect(0, 0, 10, 10), cv::Rect(20, 20, 10, 10), cv::Rect(40, 40, 10, 10)},
            {0.7f, 0.9f, 0.8f},
            0.1f,
            0.5f,
            1.0f,
            2,
            "top_k"
    );

    compare_case(
            {cv::Rect(0, 0, 10, 10), cv::Rect(20, 20, 10, 10), cv::Rect(25, 20, 10, 10)},
            {0.9f, 0.8f, 0.7f},
            0.1f,
            0.6f,
            0.5f,
            0,
            "eta"
    );
}

void run_random_correctness_cases()
{
    for (const int count : {100, 1000, 8400}) {
        for (const int seed_offset : {0, 100, 200}) {
            const NmsDataset dataset = make_dataset(count, seed_offset);
            for (const float score_threshold : {0.25f, 0.5f, 0.6f}) {
                for (const float nms_threshold : {0.45f, 0.5f}) {
                    for (const float eta : {1.0f, 0.9f}) {
                        for (const int top_k : {0, 64}) {
                            compare_case(
                                    dataset.boxes,
                                    dataset.scores,
                                    score_threshold,
                                    nms_threshold,
                                    eta,
                                    top_k,
                                    "random count=" + std::to_string(count)
                                            + " seed_offset=" + std::to_string(seed_offset)
                                            + " score=" + std::to_string(score_threshold)
                                            + " nms=" + std::to_string(nms_threshold)
                                            + " eta=" + std::to_string(eta)
                                            + " top_k=" + std::to_string(top_k)
                            );
                        }
                    }
                }
            }
        }
    }
}

void validate_baas_matches_opencv()
{
    run_explicit_correctness_cases();
    run_random_correctness_cases();
    std::cout << "BAAS NMS matches OpenCV dnn::NMSBoxes for all benchmark correctness cases.\n";
}

void set_common_counters(benchmark::State& state, int count, int kept_count)
{
    state.SetItemsProcessed(state.iterations() * count);
    state.counters["boxes"] = count;
    state.counters["score_threshold"] = static_cast<double>(state.range(1)) / 100.0;
    state.counters["nms_threshold"] = static_cast<double>(state.range(2)) / 100.0;
    state.counters["kept"] = kept_count;
}

void BM_BAAS_NMS(benchmark::State& state)
{
    const int count = static_cast<int>(state.range(0));
    const float score_threshold = static_cast<float>(state.range(1)) / 100.0f;
    const float nms_threshold = static_cast<float>(state.range(2)) / 100.0f;
    const NmsDataset& dataset = dataset_for_count(count);

    std::vector<int> indices;
    indices.reserve(dataset.boxes.size());

    for (auto _ : state) {
        baas::BAASImageUtil::nms_boxes(dataset.boxes, dataset.scores, score_threshold, nms_threshold, indices);
        benchmark::DoNotOptimize(indices.data());
        benchmark::DoNotOptimize(indices.size());
        benchmark::ClobberMemory();
    }

    set_common_counters(state, count, static_cast<int>(indices.size()));
}

void BM_OPENCV_DNN_NMS(benchmark::State& state)
{
    const int count = static_cast<int>(state.range(0));
    const float score_threshold = static_cast<float>(state.range(1)) / 100.0f;
    const float nms_threshold = static_cast<float>(state.range(2)) / 100.0f;
    const NmsDataset& dataset = dataset_for_count(count);

    std::vector<int> indices;
    indices.reserve(dataset.boxes.size());

    for (auto _ : state) {
        cv::dnn::NMSBoxes(dataset.boxes, dataset.scores, score_threshold, nms_threshold, indices);
        benchmark::DoNotOptimize(indices.data());
        benchmark::DoNotOptimize(indices.size());
        benchmark::ClobberMemory();
    }

    set_common_counters(state, count, static_cast<int>(indices.size()));
}

BENCHMARK(BM_BAAS_NMS)
        ->ArgsProduct({{100, 1000, 8400}, {25, 50, 60}, {45, 50}})
        ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_OPENCV_DNN_NMS)
        ->ArgsProduct({{100, 1000, 8400}, {25, 50, 60}, {45, 50}})
        ->Unit(benchmark::kMillisecond);

} // namespace

int main(int argc, char** argv)
{
    try {
        validate_baas_matches_opencv();
    } catch (const std::exception& e) {
        std::cerr << "baas_nms_benchmark correctness check failed: " << e.what() << '\n';
        return 1;
    }

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
