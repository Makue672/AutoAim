#ifndef ARMOR_DEF_H
#define ARMOR_DEF_H

#include <opencv2/opencv.hpp>

// 敌方颜色枚举
enum class EnemyColor { RED, BLUE };

// 灯条结构体
struct LightBar {
	cv::RotatedRect rect;// 旋转矩形
	float angle;         // 旋转角度
	cv::Point2f center;  // 中心点

    LightBar() = default;
    LightBar(const cv::RotatedRect& r) : rect(r), angle(r.angle), center(r.center) {}
};

// 装甲板结构体
struct Armor {
    LightBar left_light;
    LightBar right_light;
    cv::Point2f center;// 图像中心点
    cv::Mat number_img;// 数字区域二值图
    int number;        // 暂时默认为0

	Armor() : number(0) {}// 默认构造函数
};

#endif // ARMOR_DEF_H