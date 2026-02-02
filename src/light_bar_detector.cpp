#include "light_bar_detector.h"

LightBarDetector::LightBarDetector() {
	color_threshold_ = 100; // 二值化阈值，需要根据光照情况调整
}

std::vector<LightBar> LightBarDetector::detect(const cv::Mat& frame, EnemyColor enemy_color) {
    std::vector<LightBar> light_bars;
    cv::Mat gray_color;
    std::vector<cv::Mat> channels;

    cv::split(frame, channels);

    // 颜色差分预处理：增强目标颜色区域对比度
    if (enemy_color == EnemyColor::RED) {
        cv::subtract(channels[2], channels[0], gray_color); // R - B
    }
    else {
        cv::subtract(channels[0], channels[2], gray_color); // B - R
    }

    cv::Mat mask_color;
	// 提取颜色区域（灯条边缘）
    cv::threshold(gray_color, mask_color, 30, 255, cv::THRESH_BINARY);

	// 亮度预处理
    cv::Mat gray_brightness;
    cv::cvtColor(frame, gray_brightness, cv::COLOR_BGR2GRAY);

    cv::Mat mask_white;
	// 提取过曝区域
    cv::threshold(gray_brightness, mask_white, 240, 255, cv::THRESH_BINARY);

    // 过曝区域验证
	// 形态学膨胀：把颜色边缘膨胀使其能够与过曝区域接触
    cv::Mat mask_color_dilate;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 5)); // 竖向核
    cv::dilate(mask_color, mask_color_dilate, element);

	// 取交集（有效白色区域）
    cv::Mat mask_valid_white;
    cv::bitwise_and(mask_white, mask_color_dilate, mask_valid_white);

	// 融合两部分结果
    cv::Mat binary_final;
    cv::bitwise_or(mask_color, mask_valid_white, binary_final);

    // 闭运算：把颜色边缘和白色中心可能存在的缝隙填上
    cv::morphologyEx(binary_final, binary_final, cv::MORPH_CLOSE, element);


	// 轮廓的提取与筛选
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary_final, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 筛选灯条
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) < 5) continue; // 过滤噪点

        cv::RotatedRect r_rect = cv::minAreaRect(contour);
        cv::Size2f size = r_rect.size;

        // 统一长宽定义（长边为length）
        float length = std::max(size.width, size.height);
        float width = std::min(size.width, size.height);


        // 几何特征筛选
        float ratio = length / width;
		// 通过长宽比筛选灯条
        if (ratio < 0.8 || ratio > 15.0) continue;

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