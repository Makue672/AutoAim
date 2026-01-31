#ifndef CAMERA_PARAM_H
#define CAMERA_PARAM_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

class CameraParam {
public:
    cv::Mat camera_matrix;    // 内参矩阵
    cv::Mat dist_coeffs;      // 畸变系数
    int width, height;

    CameraParam() = default;

    // 加载yaml文件
    bool loadFromYaml(const std::string& path) {
        cv::FileStorage fs(path, cv::FileStorage::READ);
        if (!fs.isOpened()) {
            return false;
        }

        fs["image_width"] >> width;
        fs["image_height"] >> height;

        std::vector<double> matrix_data, dist_data;
        fs["camera_matrix"]["data"] >> matrix_data;
        fs["distortion_coefficients"]["data"] >> dist_data;

        // 将vector数据转化为Mat
        camera_matrix = cv::Mat(3, 3, CV_64F, matrix_data.data()).clone();
        dist_coeffs = cv::Mat(1, 5, CV_64F, dist_data.data()).clone();

        fs.release();
        return true;
    }
};

#endif