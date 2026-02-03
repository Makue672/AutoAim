#include "armor_matcher.h"
#include <algorithm> 
#include <cmath>     

std::vector<Armor> ArmorMatcher::match(const std::vector<LightBar>& light_bars, const cv::Mat& frame, NumberRecognizer* recognizer) {
    std::vector<Armor> armors;
    std::vector<MatchCandidate> candidates;

    // 遍历所有可能的两两组合，计算评分
    for (size_t i = 0; i < light_bars.size(); ++i) {
        for (size_t j = i + 1; j < light_bars.size(); ++j) {
            const auto& lb1 = light_bars[i];
            const auto& lb2 = light_bars[j];

            if (isArmor(lb1, lb2)) {
				// 检查两个灯条之间是否有其他灯条存在
                bool has_light_inside = false;

                // 定义这个潜在装甲板的 X 轴范围
                float x_min = std::min(lb1.center.x, lb2.center.x) + 5;
                float x_max = std::max(lb1.center.x, lb2.center.x) - 5;
                // 定义 Y 轴范围
                float y_min = std::min(lb1.center.y, lb2.center.y) - 20;
                float y_max = std::max(lb1.center.y, lb2.center.y) + 20;

                for (size_t k = 0; k < light_bars.size(); k++) {
                    if (k == i || k == j) continue; // 跳过自己

                    const auto& test_bar = light_bars[k];
                    // 如果第三个灯条的中心点落在这个矩形范围内
                    if (test_bar.center.x > x_min && test_bar.center.x < x_max &&
                        test_bar.center.y > y_min && test_bar.center.y < y_max) {
                        has_light_inside = true;
                        break;
                    }
                }

                if (has_light_inside) continue; // 中间有灯条直接跳过，不匹配
                // 计算匹配误差

                // 角度差误差
                float angle_diff = std::abs(lb1.angle - lb2.angle);

                // 长度差比率误差
                float len1 = std::max(lb1.rect.size.width, lb1.rect.size.height);
                float len2 = std::max(lb2.rect.size.width, lb2.rect.size.height);
                float len_diff_ratio = std::abs(len1 - len2) / std::max(len1, len2);

                // Y轴高度差比率误差 (防止错位)
                float avg_len = (len1 + len2) / 2.0f;
                float y_diff_ratio = std::abs(lb1.center.y - lb2.center.y) / avg_len;

                // 综合评分公式：加权求和
                // 给长度差和高度差较高的权重
                float score = angle_diff + len_diff_ratio * 50 + y_diff_ratio * 100;

                candidates.push_back({ i, j, score });
            }
        }
    }

	// 按分数从小到大排序，优先选择更像装甲板的组合
    std::sort(candidates.begin(), candidates.end(), [](const MatchCandidate& a, const MatchCandidate& b) {
        return a.score < b.score;
        });

	// 记录已使用的灯条，防止重复使用
    std::vector<bool> used(light_bars.size(), false);
    
    // 贪心算法+svm验证生成最终结果
    for (const auto& c : candidates) {
        // 如果这两个灯条都还没被使用过
        if (!used[c.idx1] && !used[c.idx2]) {
            const auto& lb1 = light_bars[c.idx1];
            const auto& lb2 = light_bars[c.idx2];

            Armor armor;
            // 确保左右灯条顺序正确 (x坐标小的在左)
            if (lb1.center.x < lb2.center.x) {
                armor.left_light = lb1; armor.right_light = lb2;
            }
            else {
                armor.left_light = lb2; armor.right_light = lb1;
            }
            armor.center = (lb1.center + lb2.center) / 2.0f;

            //大小装甲板判断
            float len1 = std::max(lb1.rect.size.width, lb1.rect.size.height);
            float len2 = std::max(lb2.rect.size.width, lb2.rect.size.height);
            float avg_len = (len1 + len2) / 2.0f;
            float dis = cv::norm(lb1.center - lb2.center);

            // 计算比值：灯条间距 / 灯条平均长度
            float ratio = dis / avg_len;

            if (ratio > 2.5f) {
                armor.type = ArmorType::LARGE;
            }
            else {
                armor.type = ArmorType::SMALL;
            }
            // svm验证

            // 提取 ROI 
            cv::Mat roi = recognizer->getRoi(armor, frame);

            // 预测
            int label = recognizer->predict(roi);

			// 如果识别结果为0，说明不是有效数字
            if (label == 0) {
				continue; // 跳过,该灯条还可继续使用
            }

			armor.number = label; // 存下识别结果

            armor.number_img = roi;
            armors.push_back(armor);

            used[c.idx1] = true;
            used[c.idx2] = true;
        }
    }

    return armors;
}

// 基础筛选逻辑
bool ArmorMatcher::isArmor(const LightBar& lb1, const LightBar& lb2) {
    // 角度差筛选
    float angle_diff = std::abs(lb1.angle - lb2.angle);
    if (angle_diff > 11.0f) return false; 

    // 长度差筛选
    float len1 = std::max(lb1.rect.size.width, lb1.rect.size.height);
    float len2 = std::max(lb2.rect.size.width, lb2.rect.size.height);
    if (std::abs(len1 - len2) / std::max(len1, len2) > 0.4f) return false;

    // 距离与长度比值筛选 (Ratio)
    float dis = cv::norm(lb1.center - lb2.center);
    float avg_len = (len1 + len2) / 2.0f;
    float ratio = dis / avg_len;

	// 排除过近或过远的灯条组合
    if (ratio < 1.5f || ratio > 4.2f) return false;

    // 高度差筛选 (防止错位)
    float y_diff = std::abs(lb1.center.y - lb2.center.y);
    if (y_diff / avg_len > 0.6f) return false;

    return true;
}