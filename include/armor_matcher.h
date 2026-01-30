#ifndef ARMOR_MATCHER_H
#define ARMOR_MATCHER_H

#include "armor_def.h"
#include <vector>

class ArmorMatcher {
public:
    // 匹配灯条对
    std::vector<Armor> match(const std::vector<LightBar>& light_bars);

private:
    // 校验两个灯条是否符合装甲板几何特征
    bool isArmor(const LightBar& light_1, const LightBar& light_2);
};

#endif