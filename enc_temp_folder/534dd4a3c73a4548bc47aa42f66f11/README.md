这是一个为您量身定制的 `README.md` 文件。它涵盖了我们讨论过的所有核心功能（SVM、凸包检测、防过曝、大小装甲板判断、PnP解算等），结构清晰，适合作为项目的文档。

您可以直接将以下内容复制到项目根目录下的 `README.md` 文件中。

---

# RoboMaster Auto-Aim System (C++ / OpenCV) ??

## ?? 项目简介 (Introduction)

本项目是一个基于 **OpenCV** 和 **C++** 开发的 RoboMaster 视觉自瞄系统基础框架。
项目包含完整的**装甲板检测**、**数字识别 (SVM)**、**PnP 位姿解算**以及**调试交互**功能。针对近距离高曝光（灯条中空）、地面反光干扰以及装甲板错误匹配等常见问题进行了优化。

## ? 核心功能 (Key Features)

* **抗干扰灯条检测 (Robust LightBar Detection)**
* 采用 **颜色通道差分 (R-B / B-R)** 剔除环境白光干扰。
* 包含形态学处理与长宽比、角度筛选，有效过滤地面反光。


* **高精度装甲板匹配 (Armor Matching)**
* 引入 **Score 评分机制**，综合角度差、长度差、Y轴高度差，优先匹配最佳组合，解决“乱连线”问题。(实际效果仍然有装甲板两两匹配错误的问题，待后续优化）
* 自动识别 **大装甲板 (Big)** 与 **小装甲板 (Small)**。


* **数字识别 (SVM Number Recognition)**
* 集成 **SVM (支持向量机)** 分类器，支持1 2 3 4 7数字识别。
* 包含透视变换 (Perspective Transform) 提取标准 ROI。
* 附带独立的 **模型训练工具**，支持自动化数据集加载与训练。


* **交互式调试 (Interactive Debugging)**
* 使用 OpenCV Trackbar 实时调整 **二值化阈值** 。
* 可视化绘制：绿色灯条中心线、黄色装甲板对角线、类别与数字显示。


## ?? 项目结构 (Project Structure)

```text
AutoAimProject/
├── CMakeLists.txt              # CMake 构建脚本
├── camera_info.yaml            # 相机内参文件 (PnP解算必需)
├── svm_model.xml               # 训练好的 SVM 模型文件
├── include/                    # 头文件
│   ├── armor_def.h             # 基础结构体 (Armor, LightBar, EnemyColor)
│   ├── light_bar_detector.h    # 灯条检测类
│   ├── armor_matcher.h         # 装甲板匹配类
│   ├── number_recognizer.h     # 数字识别类 (SVM)
│   ├── pose_solver.h           # PnP 解算类
│   └── camera_param.h          # 相机参数读取
├── src/                        # 源代码
│   ├── main.cpp                # 主程序 (包含调试逻辑与绘图)
│   ├── light_bar_detector.cpp  # 实现凸包检测与筛选
│   ├── armor_matcher.cpp       # 实现评分匹配与大小板判断
│   └── number_recognizer.cpp   # 实现 SVM 推理

```

## ??? 编译与运行 (Build & Run)

### 依赖环境

* **OpenCV** (建议 3.4.x 或 4.x，需包含 `opencv_ml` 模块)
* **CMake** (>= 3.10)
* **C++ Compiler** (支持 C++17)

### 编译步骤

```bash
mkdir build
cd build
cmake ..
# Windows (Visual Studio): 生成 .sln 后打开编译，或直接使用 "cmake --build ."

### 运行

1. 确保 `camera_info.yaml` 和 `svm_model.xml` 的绝对路径正确。
2. 注意更改视频源（摄像头索引或视频文件路径）在 `main.cpp` 中。
3. 注意在注释表明处设置敌方颜色 (Red/Blue)。
4. 若要使用二值化窗口进行调试，请更改 `main.cpp` 中的相关代码的通道相减代码。
5. 运行可执行文件。


## ?? 操作指南 (Usage)

### 1. 实时调试窗口 (Debug Control)

程序运行时会弹出一个控制面板：

* **Threshold**: 滑动调整二值化阈值。
同时可观察二值化窗口效果，找到最佳阈值以确保灯条完整且无过多噪点。



### 2. 数据采集模式

在主窗口激活状态下：

* 按下键盘数字键 **`1` ~ `5**`: 将当前识别到的装甲板数字 ROI 保存到 `data/` 文件夹。
* 文件名格式自动生成为：`1_时间戳.jpg`，方便训练工具自动读取标签。

### 3. 按键控制

* `Esc`: 退出程序。


## ?? 视觉算法流程 (Pipeline)

1. **预处理 (Pre-processing)**: 图像分离通道 -> 颜色通道相减 (R-B) -> 低阈值二值化。
2. **灯条提取 (LightBar Detection)**:
* 形态学竖向膨胀 (连接断裂边缘)。
* 筛选: 面积、长宽比、角度 (去除横向反光)。


3. **装甲板匹配 (Armor Matching)**:
* 计算所有两两灯条组合的 Score (角度差 + 长度差 + 高度差)。
* 按 Score 排序，贪心算法择优匹配。
* 根据宽高比判定 `LARGE` / `SMALL` 装甲板。


4. **数字识别 (Number Recognition)**: 透视变换矫正 -> 二值化 -> SVM 分类。
5. **结果解算 (PnP)**: 结合相机内参计算 3D 距离。

---

**Author:** [Makue672]
**Date:** 2026-02# AutoAim