#ifndef NUMBER_RECOGNIZER_H
#define NUMBER_RECOGNIZER_H

#include "armor_def.h"
#include <opencv2/ml.hpp>

class NumberRecognizer {
public:
    NumberRecognizer();
    // 提取数字区域 ROI 并进行预处理
    void process(std::vector<Armor>& armors, const cv::Mat& src);
    // 提取单张装甲板的数字区域 ROI 
    cv::Mat getRoi(const Armor& armor, const cv::Mat& src);
    // 对单张 ROI 进行 SVM 预测，返回数字标签
    int predict(const cv::Mat& roi);

private:
    cv::Ptr<cv::ml::SVM> svm_; // SVM指针
};
#endif