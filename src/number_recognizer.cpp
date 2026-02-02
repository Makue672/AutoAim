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

// 提取 ROI
cv::Mat NumberRecognizer::getRoi(const Armor& armor, const cv::Mat& src) {
    // 目标ROI大小（标准SVM输入大小）
    const int roi_size = 32;
    std::vector<cv::Point2f> dst_pts = {
        {0, 0}, {(float)roi_size, 0},
        {(float)roi_size, (float)roi_size}, {0, (float)roi_size}
    };

    // 获取左右灯条的四个点
    cv::Point2f l_pts[4], r_pts[4];
    armor.left_light.rect.points(l_pts);
    armor.right_light.rect.points(r_pts);

	// Y坐标排序
    auto sort_y = [](const cv::Point2f& a, const cv::Point2f& b) { return a.y < b.y; };
    std::sort(std::begin(l_pts), std::end(l_pts), sort_y);
    std::sort(std::begin(r_pts), std::end(r_pts), sort_y);

    // 定义源图像上的四个关键点
    cv::Point2f src_pts[4];

    // 四个角点
    // 左上
    src_pts[0] = (l_pts[0] + l_pts[1]) / 2.0f;
    // 右上
    src_pts[1] = (r_pts[0] + r_pts[1]) / 2.0f;
    // 右下
    src_pts[2] = (r_pts[2] + r_pts[3]) / 2.0f;
    // 左下
    src_pts[3] = (l_pts[2] + l_pts[3]) / 2.0f;

    // 计算当前框的中心
    cv::Point2f center = (src_pts[0] + src_pts[1] + src_pts[2] + src_pts[3]) / 4.0f;

	float scale_height = 2.00f; // 高度放大比例
	float scale_width = 0.70f;  // 宽度缩小比例

	// 调整四个点以改变 ROI 大小
    for (int i = 0; i < 4; i++) {
        cv::Point2f vec = src_pts[i] - center;
        src_pts[i].x = center.x + vec.x * scale_width;
        src_pts[i].y = center.y + vec.y * scale_height;
    }

    // 透视变换
    cv::Mat M = cv::getPerspectiveTransform(src_pts, dst_pts.data());
    cv::Mat roi;
    cv::warpPerspective(src, roi, M, cv::Size(roi_size, roi_size));

    // 二值化处理
    cv::cvtColor(roi, roi, cv::COLOR_BGR2GRAY);
    cv::threshold(roi, roi, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    return roi;
}

// SVM 预测
int NumberRecognizer::predict(const cv::Mat& roi) {
    if (roi.empty()) return 0;
    if (!svm_ || svm_->empty()) return 0; // 没模型就返回0

    // 转换为float向量
    cv::Mat sample = roi.reshape(1, 1);
    sample.convertTo(sample, CV_32F);

    float result = svm_->predict(sample);
    return static_cast<int>(result);
}

void NumberRecognizer::process(std::vector<Armor>& armors, const cv::Mat& src) {
    for (auto& armor : armors) {
        // 调用 getRoi 获取图片
        cv::Mat roi = getRoi(armor, src);
        armor.number_img = roi;

        // 调用 predict 获取数字
        armor.number = predict(roi);
    }
}