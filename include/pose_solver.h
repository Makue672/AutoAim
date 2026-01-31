#ifndef POSE_SOLVER_H
#define POSE_SOLVER_H

#include "armor_def.h"
#include "camera_param.h"

class PoseSolver {
public:
    PoseSolver(const CameraParam& camera_param) : cam_param_(camera_param) {
        // 定义小装甲板的世界坐标系 (单位: m)
        // 假设装甲板中心为原点，宽 135mm，高 55mm
        float half_w = 0.135f / 2.0f;
        float half_h = 0.055f / 2.0f;

        // 3D点顺序：左上 -> 右上 -> 右下 -> 左下
        object_points_.emplace_back(-half_w, -half_h, 0);
        object_points_.emplace_back(half_w, -half_h, 0);
        object_points_.emplace_back(half_w, half_h, 0);
        object_points_.emplace_back(-half_w, half_h, 0);
    }

    void solve(Armor& armor) {
        std::vector<cv::Point2f> image_points;

        // 获取左右灯条的四个顶点
        cv::Point2f l_pts[4], r_pts[4];
        armor.left_light.rect.points(l_pts);
        armor.right_light.rect.points(r_pts);

        // 按Y轴排序，区分灯条的上下顶点
        auto sort_y = [](const cv::Point2f& a, const cv::Point2f& b) { return a.y < b.y; };
        std::sort(std::begin(l_pts), std::end(l_pts), sort_y);
        std::sort(std::begin(r_pts), std::end(r_pts), sort_y);

        // 构造图像点顺序：左上 -> 右上 -> 右下 -> 左下
        // 取灯条顶点
        // 0,1 为上部点; 2,3 为下部点
        cv::Point2f lu = (l_pts[0] + l_pts[1]) / 2.0f;
        cv::Point2f ld = (l_pts[2] + l_pts[3]) / 2.0f;
        cv::Point2f ru = (r_pts[0] + r_pts[1]) / 2.0f;
        cv::Point2f rd = (r_pts[2] + r_pts[3]) / 2.0f;

        image_points.push_back(lu);
        image_points.push_back(ru);
        image_points.push_back(rd);
        image_points.push_back(ld);

        // 解算 PnP
        cv::Mat rvec, tvec;
        bool success = cv::solvePnP(object_points_, image_points,
            cam_param_.camera_matrix, cam_param_.dist_coeffs,
            rvec, tvec);

        if (success) {
            armor.rvec = rvec;
            armor.tvec = tvec;
            // 计算直线距离
            armor.distance = std::sqrt(tvec.at<double>(0) * tvec.at<double>(0) +
                tvec.at<double>(1) * tvec.at<double>(1) +
                tvec.at<double>(2) * tvec.at<double>(2));
        }
    }

private:
    CameraParam cam_param_;
    std::vector<cv::Point3f> object_points_;
};

#endif