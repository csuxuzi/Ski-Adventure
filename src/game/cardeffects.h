#ifndef CARDEFFECTS_H
#define CARDEFFECTS_H

#include <QString>

// 1. 定义卡牌效果的类型
enum class CardEffectType {
    None,
    IncreaseSpeed,
    DecreaseGravity,
    IncreaseJump,
    SlowDownAvalanche
    // 未来可以在这里添加更多效果类型
};

// 2. 定义一个结构体来完整描述一张卡片
struct CardData {
    CardEffectType type = CardEffectType::None;
    QString title = "卡牌标题";
    QString description = "这里是卡牌效果的详细描述。";
    // 未来可以添加更多数据，比如效果的数值（加多少速）、持续时间等
    float value = 0.0f;
};

#endif // CARDEFFECTS_H
