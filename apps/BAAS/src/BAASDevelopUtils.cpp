//
// Created by pc on 2024/5/31.
//

#include "BAASDevelopUtils.h"

#include <random>

#include "BAASGlobals.h"
#include "utils/BAASImageUtil.h"

using namespace std;
using namespace cv;
using namespace std::chrono;

BAAS_NAMESPACE_BEGIN

void BAASDevelopUtils::shotStudentSkill(
        BAAS* baas,
        const string &name,
        const BAASRectangle &r,
        const int type
)
{
    std::filesystem::path temp = "D:\\github\\BAAS_Cpp\\apps\\BAAS\\resource\\image\\CN\\zh-cn\\skill";
    if (type == SKILL_FULL) {
        temp = temp / "active";
    } else if (type == SKILL_LEFT) {
        temp = temp / "l_inactive";
    } else if (type == SKILL_RIGHT) {
        temp = temp / "r_inactive";
    }
    Mat image;
    baas->get_latest_screenshot(image);
    Mat im = BAASImageUtil::crop(image, r);
    BAASImageUtil::save(im, name, temp.string(), true);
}

void BAASDevelopUtils::extract_image_rgb_range(
        const cv::Mat &img,
        const string &name,
        const BAASRectangle &r,
        const Scalar &min_,
        const Scalar &max_,
        const uint8_t cut_edge
)
{
    Mat im = img.clone();
    BAASImageUtil::filter_region_rgb(im, r, min_, max_);
    BAASRectangle region = r;
    if (cut_edge) {
        vector<int> rg;
        BAASImageUtil::crop_edge(im, 0b1111, rg, {0, 0, 0}, {0, 0, 0}, im);
        BAASGlobalLogger->BAASInfo(
                "crop region: " + to_string(rg[0]) + " " + to_string(rg[1]) + " " + to_string(rg[2]) + " " +
                to_string(rg[3]));
        region.ul
              .x += rg[2];
        region.ul
              .y += rg[0];
        region.lr
              .x -= rg[3];
        region.lr
              .y -= rg[1];
    }
    BAASGlobalLogger->BAASInfo("Region: " + region.to_string());
    BAASGlobalLogger->BAASInfo("Shape: " + to_string(im.cols) + "x" + to_string(im.rows));
    imwrite(name + ".png", im);
}

void BAASDevelopUtils::fight_screenshot_extract(BAAS* baas, const screenshot_extract_params& params)
{
    std::filesystem::path folder = params.img_folder;
    if(!std::filesystem::exists(folder)) {
        BAASGlobalLogger->BAASError("Image Extract Folder does not exist: " + folder.string());
        throw PathError("Expected Path Not Exist.");
    }

    int next_index = get_next_image_index(folder);

    baas->solve_procedure("UI-FROM_PAGE_formation_TO_PAGE_fighting", true);

    auto start_time_ms = BAASChronoUtil::getCurrentTimeMS();

    if(params.pre_wait > 0)
        BAASChronoUtil::sleepMS(int(params.pre_wait * 1000));

    std::vector<long long> timestamps;

    if (params.random) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, int(params.duration * 1000));
        for (int i = 0; i < params.img_count; ++i) {
            timestamps.push_back(start_time_ms + dis(gen));
        }
    } else {
        auto itv = static_cast<long long>(params.interval * 1000);
        for (int i = 0; i < params.img_count; ++i) {
            timestamps.push_back(start_time_ms + i * itv);
        }
    }

    sort(timestamps.begin(), timestamps.end());


    for (int i = 0; i < params.img_count; ++i) {
        if(BAASChronoUtil::getCurrentTimeMS() < timestamps[i]) {
            BAASChronoUtil::sleepMS(int(timestamps[i] - BAASChronoUtil::getCurrentTimeMS()));
        }

        baas->i_update_screenshot_array();
        cv::Mat image;
        baas->get_latest_screenshot(image);

        std::filesystem::path filename = folder / (std::to_string(next_index++) + ".png");
        cv::imwrite(filename.string(), image);
        BAASGlobalLogger->BAASInfo(filename.filename().string() + " saved. Left: " + std::to_string(params.img_count - i));

    }

}

int BAASDevelopUtils::get_next_image_index(const filesystem::path& folder)
{
    std::regex pattern(R"((\d+)\.png)");
    int max_index = -1;

    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (!entry.is_regular_file())
            continue;

        std::smatch match;
        std::string filename = entry.path().filename().string();
        if (std::regex_match(filename, match, pattern)) {
            int index = std::stoi(match[1]);
            max_index = max(max_index, index);
        }
    }

    return max_index + 1;  // 返回下一个可用编号
}

BAAS_NAMESPACE_END
