#include "number_recognizer.h"

NumberRecognizer::NumberRecognizer() {
    // 尝试加载模型
    try {
        svm_ = cv::ml::SVM::load("C:/Users/March/Desktop/AutoAim/svm_model.xml");
    }
    catch (...) {
        std::cerr << "Warning: svm_model.xml not found! Recognition will fail." << std::endl;
        svm_ = nullptr;
    }
}

void NumberRecognizer::process(std::vector<Armor>& armors, const cv::Mat& src) {
    // 目标ROI大小（标准SVM输入大小）
    const int roi_size = 32;
    std::vector<cv::Point2f> dst_pts = {
        {0, 0}, {(float)roi_size, 0},
        {(float)roi_size, (float)roi_size}, {0, (float)roi_size}
    };

    for (auto& armor : armors) {
        // 获取透视变换的源点（取左右灯条的角点）
        cv::Point2f l_pts[4], r_pts[4];
        armor.left_light.rect.points(l_pts);
        armor.right_light.rect.points(r_pts);

        // 简单排序：找出左灯条的上、下点和右灯条的上、下点
        // 使用Y轴排序
        auto sort_y = [](const cv::Point2f& a, const cv::Point2f& b) { return a.y < b.y; };
        std::sort(std::begin(l_pts), std::end(l_pts), sort_y);
        std::sort(std::begin(r_pts), std::end(r_pts), sort_y);

        // 定义装甲板四个顶点：左上、右上、右下、左下
        cv::Point2f src_pts[4];
        src_pts[0] = (l_pts[0] + l_pts[1]) / 2.0f; // 左灯条上中心
        src_pts[1] = (r_pts[0] + r_pts[1]) / 2.0f; // 右灯条上中心
        src_pts[2] = (r_pts[2] + r_pts[3]) / 2.0f; // 右灯条下中心
        src_pts[3] = (l_pts[2] + l_pts[3]) / 2.0f; // 左灯条下中心

        // 计算当前框的中心
        cv::Point2f center = (src_pts[0] + src_pts[1] + src_pts[2] + src_pts[3]) / 4.0f;

        float scale_height = 2.00f; //
        float scale_width = 0.70f;  //

        for (int i = 0; i < 4; i++) {
            cv::Point2f vec = src_pts[i] - center;
            src_pts[i].x = center.x + vec.x * scale_width;
            src_pts[i].y = center.y + vec.y * scale_height;
        }

        // 计算透视矩阵并应用
        cv::Mat M = cv::getPerspectiveTransform(src_pts, dst_pts.data());
        cv::Mat roi;
        cv::warpPerspective(src, roi, M, cv::Size(roi_size, roi_size));

        // 预处理：转灰度+二值化
        cv::cvtColor(roi, roi, cv::COLOR_BGR2GRAY);
        cv::threshold(roi, roi, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        armor.number_img = roi;
        
        // SVM 预测
        if (svm_ && !svm_->empty()) {
            cv::Mat sample = roi.reshape(1, 1); // 展平
            sample.convertTo(sample, CV_32F);   // 转化为float
            float result = svm_->predict(sample);
            armor.number = static_cast<int>(result);
        }
        else {
            armor.number = 0;
        }
    }
}