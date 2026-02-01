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

	// 形态学膨胀，连接断开的部分（以便识别远处目标）
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 9));
    cv::dilate(binary_img, binary_img, element);

    // 轮廓查找
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_img, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 筛选灯条
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) < 3) continue; // 过滤噪点

        cv::RotatedRect r_rect = cv::minAreaRect(contour);
        cv::Size2f size = r_rect.size;

        // 统一长宽定义（长边为length）
        float length = std::max(size.width, size.height);
        float width = std::min(size.width, size.height);

		if (length * width > 40000) continue; // 过滤过大目标（地面反光等）

        // 几何特征筛选
        float ratio = length / width;
		// 通过长宽比筛选灯条
        if (ratio < 2.5 || ratio > 15.0) continue;

		// 角度筛选
        float angle = r_rect.angle;
        if (size.width > size.height) {
            angle += 90; // 如果宽比高大，说明矩形横着存的，要修正角度
        }
		// 只保留接近垂直的灯条
        if (std::abs(angle) > 35.0 && std::abs(angle) < 145.0) continue;

        light_bars.emplace_back(LightBar(r_rect));
    }

    return light_bars;
}

// 设置阈值方法
void LightBarDetector::setThreshold(int threshold) {
    color_threshold_ = threshold;
}

// 获取阈值方法
int LightBarDetector::getThreshold() const {
    return color_threshold_;
}