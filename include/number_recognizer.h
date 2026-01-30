#ifndef NUMBER_RECOGNIZER_H
#define NUMBER_RECOGNIZER_H

#include "armor_def.h"

class NumberRecognizer {
public:
    // 提取数字区域 ROI 并进行预处理
    void process(std::vector<Armor>& armors, const cv::Mat& src);
};

#endif