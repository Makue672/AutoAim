#ifndef ARMOR_MATCHER_H
#define ARMOR_MATCHER_H

#include "armor_def.h"
#include <vector>

struct MatchCandidate {
    size_t idx1;
    size_t idx2;
    float score; // 分数越小越好
};

class ArmorMatcher {
public:
    // 匹配灯条对
    std::vector<Armor> match(const std::vector<LightBar>& light_bars);

private:
    // 校验两个灯条是否符合装甲板几何特征
    bool isArmor(const LightBar& light_1, const LightBar& light_2);
};

#endif