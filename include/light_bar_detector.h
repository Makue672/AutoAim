#ifndef LIGHT_BAR_DETECTOR_H
#define LIGHT_BAR_DETECTOR_H

#include "armor_def.h"
#include <vector>

class LightBarDetector {
public:
    LightBarDetector();
    // 检测图像中的所有灯条
    std::vector<LightBar> detect(const cv::Mat& frame, EnemyColor enemy_color);

private:
    int color_threshold_; // 二值化阈值
};

#endif