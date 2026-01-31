#include <opencv2/opencv.hpp>
#include <iostream>
#include "light_bar_detector.h"
#include "armor_matcher.h"
#include "number_recognizer.h"
#include "camera_param.h"

int main() {
    //加载相机参数
    CameraParam cam_param;
    if (!cam_param.loadFromYaml("C:\\Users\\March\\Desktop\\my-cv-project\\camera_info.yaml")) {
        std::cerr << "Error: Cannot load camera_info.yaml" << std::endl;
        return -1;
    }
    // 初始化模块
    LightBarDetector detector;
    ArmorMatcher matcher;
    NumberRecognizer recognizer;

    // 打开视频文件(注意修改视频路径)
    cv::VideoCapture cap("C:\\Users\\March\\Desktop\\my-cv-project\\test02.mp4");
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open video file." << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true) {
		cap >> frame;             // 读取一帧
		if (frame.empty()) break; // 视频结束


        // 灯条检测(在此处更改敌军颜色)
        auto light_bars = detector.detect(frame, EnemyColor::RED);

        // 装甲板匹配
        auto armors = matcher.match(light_bars);

        // 数字ROI提取
        recognizer.process(armors, frame);

        // 可视化
        for (const auto& armor : armors) {
            // 绘制灯条
            cv::Point2f v[4];
            armor.left_light.rect.points(v);
            for (int i = 0; i < 4; i++) cv::line(frame, v[i], v[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
            armor.right_light.rect.points(v);
            for (int i = 0; i < 4; i++) cv::line(frame, v[i], v[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);

            // 绘制装甲板中心和数字（暂时是0）
            cv::circle(frame, armor.center, 5, cv::Scalar(0, 0, 255), -1);
            cv::putText(frame, std::to_string(armor.number), armor.center,
                cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 255), 2);
        }

        // 显示结果
        cv::imshow("Auto Aim", frame);

        // 显示提取出的数字ROI，用于调试
        if (!armors.empty()) {
            cv::imshow("ROI", armors[0].number_img);
        }

        if (cv::waitKey(20) == 27) break; // ESC退出
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}