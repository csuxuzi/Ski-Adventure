#ifndef CARDEFFECTS_H
#define CARDEFFECTS_H

#include <QString>
#include <functional> // 引入 functional 头文件

// 前向声明，避免循环引用
class GameScreen;
// 1. 定义卡片类别
enum class CardCategory {
    Character, // 角色相关
    Score,     // 得分相关
    Avalanche  // 雪崩相关
};

// 2. 定义卡片档次/稀有度
enum class CardRarity {
    Common,    // 普通 (白色)
    Rare,      // 稀有 (紫色)
    Legendary  // 传说 (金色)
};

// 3. 定义一个结构体来完整描述一张卡片
struct CardData {
    QString id;                 // 唯一的卡片ID，例如 "CHAR_SPEED_1"
    QString title;              // 标题，如 "肾上腺素"
    QString description;        // 描述
    CardCategory category;      // 卡片类别
    CardRarity rarity;          // 卡片稀有度

    // 【核心】统一的函数接口
    // 它是一个可调用对象 (std::function)，接受一个指向 GameScreen 的指针作为参数
    // 这样我们就可以在卡片定义时，直接绑定它应该执行的游戏逻辑
    std::function<void(GameScreen*)> applyEffect;
};

#endif // CARDEFFECTS_H
