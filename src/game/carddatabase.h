// src/game/CardDatabase.h

#ifndef CARDDATABASE_H
#define CARDDATABASE_H

#include "cardeffects.h"
#include <QList>
#include <QMap>

class CardDatabase
{
public:
    // 获取单例实例
    static CardDatabase& instance();

    // 根据类别和稀有度随机抽取一张卡片
    CardData drawCard(CardCategory category, CardRarity rarity);

private:
    // 私有构造函数，确保单例模式
    CardDatabase();

    void initializeDatabase(); // 初始化所有卡片

    // 使用 Map 来组织卡池，第一层按类别，第二层按稀有度
    QMap<CardCategory, QMap<CardRarity, QList<CardData>>> m_cardPool;
};

#endif // CARDDATABASE_H
