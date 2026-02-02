#include <opencv2/opencv.hpp>
#include <iostream>
#include "light_bar_detector.h"
#include "armor_matcher.h"
#include "number_recognizer.h"
#include "camera_param.h"
#include "pose_solver.h"

// 滑动条绑定变量
int debug_threshold = 65;

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
    PoseSolver pose_solver(cam_param); // 初始化 PnP 解算器

    // 创建调试窗口和滑动条
    cv::namedWindow("Debug Control", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("Threshold", "Debug Control", &debug_threshold, 255);

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

        // 每一帧都把滑动条的值传给detector
        detector.setThreshold(debug_threshold);

        // 灯条检测(在此处更改敌军颜色)
        auto light_bars = detector.detect(frame, EnemyColor::RED);

        // 装甲板匹配
        auto armors = matcher.match(light_bars, frame, &recognizer);


        // PnP 解算
        for (auto& armor : armors) {
            pose_solver.solve(armor);
        }

        // 可视化
        for (const auto& armor : armors) {
            // 1. 获取灯条的角点并排序，用于确定装甲板的四个顶点
            cv::Point2f l_pts[4], r_pts[4];
            armor.left_light.rect.points(l_pts);
            armor.right_light.rect.points(r_pts);

            // 按 Y 坐标排序，找出每个灯条的上下顶点
            auto sort_y = [](const cv::Point2f& a, const cv::Point2f& b) { return a.y < b.y; };
            std::sort(std::begin(l_pts), std::end(l_pts), sort_y);
            std::sort(std::begin(r_pts), std::end(r_pts), sort_y);

            // 定义装甲板的近似四个顶点
			// 左上
            cv::Point2f tl = (l_pts[0] + l_pts[1]) / 2.0f;
            // 左下
            cv::Point2f bl = (l_pts[2] + l_pts[3]) / 2.0f;
            // 右上
            cv::Point2f tr = (r_pts[0] + r_pts[1]) / 2.0f;
            // 右下
            cv::Point2f br = (r_pts[2] + r_pts[3]) / 2.0f;

			// [绘制] 画装甲板边框 (绿色，线宽2)
            cv::line(frame, tl, bl, cv::Scalar(0, 255, 0), 2); // 左灯条
            cv::line(frame, tr, br, cv::Scalar(0, 255, 0), 2); // 右灯条

            // [绘制] 画对角线交叉 (黄色，线宽2)
            cv::line(frame, tl, br, cv::Scalar(255, 255, 0), 2);
            cv::line(frame, tr, bl, cv::Scalar(255, 255, 0), 2);

            // [绘制] 构建显示文本 "type:number"
            std::string type_str = (armor.type == ArmorType::LARGE) ? "large" : "small";
            std::string text = type_str + ":" + std::to_string(armor.number);

            // 在装甲板中心上方绘制文字 (红色字体)
            cv::putText(frame, text, armor.center - cv::Point2f(0, 25),
                cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);

            // 画 PnP 距离结果
            if (armor.distance > 0) {
                std::string dist_str = std::to_string(armor.distance).substr(0, 4) + "m";
                cv::putText(frame, dist_str, armor.center + cv::Point2f(0, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
            }
        }

        //滑动条调试，实时传输阈值参数
        cv::Mat debug_bin;
        std::vector<cv::Mat> ch;
        cv::split(frame, ch);
        cv::Mat gray_temp = ch[2] - ch[0]; // 假设红色
        cv::threshold(gray_temp, debug_bin, debug_threshold, 255, cv::THRESH_BINARY);
        cv::imshow("Binary Debug", debug_bin);

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