#ifndef NUMBER_RECOGNIZER_H
#define NUMBER_RECOGNIZER_H

#include "armor_def.h"
#include <opencv2/ml.hpp>

class NumberRecognizer {
public:
    NumberRecognizer();
    // 提取数字区域 ROI 并进行预处理
    void process(std::vector<Armor>& armors, const cv::Mat& src);
private:
    cv::Ptr<cv::ml::SVM> svm_; // SVM指针
};
#endif