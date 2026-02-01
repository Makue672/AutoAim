#include "armor_matcher.h"
#include <algorithm> 
#include <cmath>     



std::vector<Armor> ArmorMatcher::match(const std::vector<LightBar>& light_bars) {
    std::vector<Armor> armors;
    std::vector<MatchCandidate> candidates;

    // 遍历所有可能的两两组合，计算评分
    for (size_t i = 0; i < light_bars.size(); ++i) {
        for (size_t j = i + 1; j < light_bars.size(); ++j) {
            const auto& lb1 = light_bars[i];
            const auto& lb2 = light_bars[j];

            if (isArmor(lb1, lb2)) {
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
                float score = angle_diff + len_diff_ratio * 100 + y_diff_ratio * 100;

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
    
    // 贪心算法生成最终结果
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

            if (ratio > 3.2f) {
                armor.type = ArmorType::LARGE;
            }
            else {
                armor.type = ArmorType::SMALL;
            }
            // ------------------------------------------------------

            armors.push_back(armor);

            // 标记这两个灯条已用，防止被其他组合重复使用
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
    if (angle_diff > 8.0f) return false; 

    // 长度差筛选
    float len1 = std::max(lb1.rect.size.width, lb1.rect.size.height);
    float len2 = std::max(lb2.rect.size.width, lb2.rect.size.height);
    if (std::abs(len1 - len2) / std::max(len1, len2) > 0.4f) return false;

    // 距离与长度比值筛选 (Ratio)
    float dis = cv::norm(lb1.center - lb2.center);
    float avg_len = (len1 + len2) / 2.0f;
    float ratio = dis / avg_len;

	// 排除过近或过远的灯条组合
    if (ratio < 1.5f || ratio > 4.3f) return false;
    if (ratio > 2.8f && ratio < 3.8f) return false;

    // 高度差筛选 (防止错位)
    float y_diff = std::abs(lb1.center.y - lb2.center.y);
    if (y_diff / avg_len > 0.6f) return false;

    return true;
}