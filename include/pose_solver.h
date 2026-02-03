#ifndef POSE_SOLVER_H
#define POSE_SOLVER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "armor_def.h"
#include "camera_param.h"

class PoseSolver {
public:
    // 接收相机参数
    PoseSolver(const CameraParam& camera_param);

    // 接收装甲板数据
    void solve(Armor& armor);

private:
    CameraParam cam_param_;

	// 预先存储两种装甲板的3D点
    std::vector<cv::Point3f> small_armor_points_;
    std::vector<cv::Point3f> big_armor_points_;
};

#endif