// src/pose_solver.cpp
#include "pose_solver.h"
#include <iostream>

// 构造函数：初始化相机参数和装甲板3D点
PoseSolver::PoseSolver(const CameraParam& camera_param) : cam_param_(camera_param) {
    // 初始化小装甲板
    double small_w = 0.135 / 2.0;
    double small_h = 0.055 / 2.0;

    small_armor_points_.emplace_back(-small_w, -small_h, 0); // 左上
    small_armor_points_.emplace_back(-small_w, small_h, 0); // 左下
    small_armor_points_.emplace_back(small_w, small_h, 0); // 右下
    small_armor_points_.emplace_back(small_w, -small_h, 0); // 右上

    // 初始化大装甲板
    double big_w = 0.230 / 2.0;
    double big_h = 0.055 / 2.0;

    big_armor_points_.emplace_back(-big_w, -big_h, 0); // 左上
    big_armor_points_.emplace_back(-big_w, big_h, 0); // 左下
    big_armor_points_.emplace_back(big_w, big_h, 0); // 右下
    big_armor_points_.emplace_back(big_w, -big_h, 0); // 右上
}

// 解算装甲板位姿
void PoseSolver::solve(Armor& armor) {
    // 提取 2D 图像点
    std::vector<cv::Point2f> image_points;
    cv::Point2f l_pts[4], r_pts[4];
    armor.left_light.rect.points(l_pts);
    armor.right_light.rect.points(r_pts);

	// 对灯条顶点按 Y 坐标排序，找出上下顶点
    auto sort_y = [](const cv::Point2f& a, const cv::Point2f& b) { return a.y < b.y; };
    std::sort(l_pts, l_pts + 4, sort_y);
    std::sort(r_pts, r_pts + 4, sort_y);

    image_points.push_back((l_pts[0] + l_pts[1]) / 2.0f); // 左上
    image_points.push_back((l_pts[2] + l_pts[3]) / 2.0f); // 左下
    image_points.push_back((r_pts[2] + r_pts[3]) / 2.0f); // 右下
    image_points.push_back((r_pts[0] + r_pts[1]) / 2.0f); // 右上

	// 选择对应的 3D 点
    const std::vector<cv::Point3f>& object_points = (armor.type == ArmorType::LARGE) ? big_armor_points_ : small_armor_points_;

    // PnP解算
    bool success = cv::solvePnP(object_points, image_points, cam_param_.camera_matrix, cam_param_.dist_coeffs, armor.rvec, armor.tvec, false, cv::SOLVEPNP_IPPE);

    if (success) {
        double x = armor.tvec.at<double>(0);
        double y = armor.tvec.at<double>(1);
        double z = armor.tvec.at<double>(2);
        armor.distance = std::sqrt(x * x + y * y + z * z);
    }
    else {
        armor.distance = 0;
    }
}