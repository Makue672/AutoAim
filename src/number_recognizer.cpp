#include <algorithm>
#include <cmath>
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
	// 初步透视变换，得到正面数字图
    const int TEMP_SIZE = 100;
    std::vector<cv::Point2f> dst_pts = {
        {0, 0},
        {(float)TEMP_SIZE, 0},
        {(float)TEMP_SIZE, (float)TEMP_SIZE},
        {0, (float)TEMP_SIZE}
    };

	// 获取灯条点并进行y坐标排序
    cv::Point2f l_pts[4], r_pts[4];
    armor.left_light.rect.points(l_pts);
    armor.right_light.rect.points(r_pts);

    auto sort_y = [](const cv::Point2f& a, const cv::Point2f& b) { return a.y < b.y; };
    std::sort(std::begin(l_pts), std::end(l_pts), sort_y);
    std::sort(std::begin(r_pts), std::end(r_pts), sort_y);

    // 计算角点
    cv::Point2f src_pts[4];
    src_pts[0] = (l_pts[0] + l_pts[1]) / 2.0f;
    src_pts[1] = (r_pts[0] + r_pts[1]) / 2.0f;
    src_pts[2] = (r_pts[2] + r_pts[3]) / 2.0f;
    src_pts[3] = (l_pts[2] + l_pts[3]) / 2.0f;

    // 适度扩张
	float expand_h = 0.8f; // 高度扩张比例
	float expand_w = 0.1f; // 宽度收缩比例

    cv::Point2f vec_up = src_pts[0] - src_pts[3];
    cv::Point2f vec_right = src_pts[1] - src_pts[0];

    src_pts[0] -= vec_right * expand_w - vec_up * expand_h;
    src_pts[1] += vec_right * expand_w + vec_up * expand_h;
    src_pts[3] -= vec_right * expand_w + vec_up * expand_h;
    src_pts[2] += vec_right * expand_w - vec_up * expand_h;

    // 执行透视变换
    cv::Mat M = cv::getPerspectiveTransform(src_pts, dst_pts.data());
    cv::Mat warp_roi;
    cv::warpPerspective(src, warp_roi, M, cv::Size(TEMP_SIZE, TEMP_SIZE));

    // 寻找中心最亮的白色物体
    cv::Mat gray_roi, binary_roi;
    cv::cvtColor(warp_roi, gray_roi, cv::COLOR_BGR2GRAY);
    cv::threshold(gray_roi, binary_roi, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_roi, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) return cv::Mat(); // 全黑，匹配错误

    // 筛选最佳轮廓
    int best_idx = -1;
    float best_score = -1.0f;
    cv::Point2f img_center(TEMP_SIZE / 2.0f, TEMP_SIZE / 2.0f);

    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);

		// 面积过滤
        if (area < 150 || area > 6000) continue;

        cv::Rect rect = cv::boundingRect(contours[i]);

        // 位置过滤：计算轮廓中心到图像中心的距离
        cv::Point2f center(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f);
        float dist = cv::norm(center - img_center);

        // 太远则可能时灯条
        if (dist > 30.0f) continue;

        // 形状过滤：不是扁的
        float hw_ratio = (float)rect.height / (float)rect.width;
        if (hw_ratio < 0.8f) continue; 

        // 评分：面积越大越好，距离中心越近越好
        float score = (float)area - dist * 10.0f;

        if (score > best_score) {
            best_score = score;
            best_idx = i;
        }
    }

    // 如果没找到合适的轮廓，说明匹配的是空地
    if (best_idx == -1) return cv::Mat();

    // 裁剪与补边，找到最佳轮廓的包围框
    cv::Rect best_rect = cv::boundingRect(contours[best_idx]);

    // 从二值图中扣出这个数字
    cv::Mat digit_roi = binary_roi(best_rect);

    // 保持比例缩放

    int max_side = std::max(best_rect.width, best_rect.height);
    int pad_top = (max_side - best_rect.height) / 2;
    int pad_bottom = max_side - best_rect.height - pad_top;
    int pad_left = (max_side - best_rect.width) / 2;
    int pad_right = max_side - best_rect.width - pad_left;

    cv::Mat square_roi;
    cv::copyMakeBorder(digit_roi, square_roi, pad_top, pad_bottom, pad_left, pad_right, cv::BORDER_CONSTANT, cv::Scalar(0));

	// 最终缩放到 32x32
    cv::Mat final_roi;
    cv::resize(square_roi, final_roi, cv::Size(32, 32));

    return final_roi;
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