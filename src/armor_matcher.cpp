#include "armor_matcher.h"

std::vector<Armor> ArmorMatcher::match(const std::vector<LightBar>& light_bars) {
    std::vector<Armor> armors;

    // 遍历所有灯条组合
    for (size_t i = 0; i < light_bars.size(); ++i) {
        for (size_t j = i + 1; j < light_bars.size(); ++j) {
            const auto& lb1 = light_bars[i];
            const auto& lb2 = light_bars[j];

            if (isArmor(lb1, lb2)) {
                Armor armor;
                // 确定左右灯条
				if (lb1.center.x < lb2.center.x) {   // x轴坐标较小为左灯条
                    armor.left_light = lb1;
                    armor.right_light = lb2;
                }
                else {
                    armor.left_light = lb2;
                    armor.right_light = lb1;
                }
                armor.center = (lb1.center + lb2.center) / 2.0f;
                armors.push_back(armor);
            }
        }
    }
    return armors;
}

bool ArmorMatcher::isArmor(const LightBar& lb1, const LightBar& lb2) {
    // 角度平行度检查
    float angle_diff = std::abs(lb1.angle - lb2.angle);
    if (angle_diff > 10.0f) return false;

    // 长度相似度检查
    float len1 = std::max(lb1.rect.size.width, lb1.rect.size.height);
    float len2 = std::max(lb2.rect.size.width, lb2.rect.size.height);
    float len_diff_ratio = std::abs(len1 - len2) / std::max(len1, len2);
    if (len_diff_ratio > 0.5f) return false;

    // 灯条间距与灯条长度比值检查 (装甲板通常是扁平的)
    float dis = cv::norm(lb1.center - lb2.center);
    float avg_len = (len1 + len2) / 2.0f;
    float ratio = dis / avg_len;

    // 小装甲板比例约为 2.0~3.5，大装甲板约为 4.0~5.0
    if (ratio < 0.8f || ratio > 5.0f) return false;

    return true;
}