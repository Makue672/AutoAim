#include "light_bar_detector.h"

LightBarDetector::LightBarDetector() {
	color_threshold_ = 100; // 二值化阈值，需要根据光照情况调整
}

std::vector<LightBar> LightBarDetector::detect(const cv::Mat& frame, EnemyColor enemy_color) {
    std::vector<LightBar> light_bars;
    cv::Mat gray_img, binary_img;
    std::vector<cv::Mat> channels;

    cv::split(frame, channels);

    // 颜色差分预处理：增强目标颜色区域对比度
    if (enemy_color == EnemyColor::RED) {
        cv::subtract(channels[2], channels[0], gray_img); // R - B
    }
    else {
        cv::subtract(channels[0], channels[2], gray_img); // B - R
    }

    // 二值化
    cv::threshold(gray_img, binary_img, color_threshold_, 255, cv::THRESH_BINARY);

    // 轮廓查找
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 筛选灯条
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) < 20) continue; // 过滤噪点

        cv::RotatedRect r_rect = cv::minAreaRect(contour);
        cv::Size2f size = r_rect.size;

        // 统一长宽定义（长边为length）
        float length = std::max(size.width, size.height);
        float width = std::min(size.width, size.height);

        // 几何特征筛选
        float ratio = length / width;
        bool is_vertical = std::abs(r_rect.angle) < 45.0 || std::abs(r_rect.angle) > 135.0; // 简单角度过滤

        if (ratio > 1.5 && ratio < 12.0) {
            light_bars.emplace_back(LightBar(r_rect));
        }
    }

    return light_bars;
}