// src/game/CardDatabase.cpp

#include "CardDatabase.h"
#include "ui/GameScreen.h" // 需要包含 GameScreen 来调用其成员
#include "game/Player.h"
#include "game/Avalanche.h"
#include <QRandomGenerator>

// --- 单例实现 ---
CardDatabase& CardDatabase::instance()
{
    static CardDatabase db;
    return db;
}

// --- 抽卡逻辑 ---
CardData CardDatabase::drawCard(CardCategory category, CardRarity rarity)
{
    const auto& rarityPool = m_cardPool[category][rarity];
    if (rarityPool.isEmpty()) {
        // 如果指定卡池为空，返回一个默认的空卡片
        return {"EMPTY", "空", "此卡池无卡片", category, rarity, nullptr};
    }
    int randomIndex = QRandomGenerator::global()->bounded(rarityPool.size());
    return rarityPool[randomIndex];
}

// --- 私有构造函数，在这里初始化整个卡池 ---
CardDatabase::CardDatabase()
{
    initializeDatabase();
}

// --- 【核心】初始化所有卡片数据 ---
void CardDatabase::initializeDatabase()
{
    // =================================================================
    // 角色 (Character) 相关卡池
    // =================================================================

    // -- 普通 (Common) --
    m_cardPool[CardCategory::Character][CardRarity::Common].append({
        "CHAR_SPEED_1", "轻快步伐", "永久提升 10.5 基础滑行速度。",
        CardCategory::Character, CardRarity::Common,
        [](GameScreen* gs){ gs->getPlayer()->applySpeedBonus(10.5f); } // 直接绑定逻辑
    });
    m_cardPool[CardCategory::Character][CardRarity::Common].append({
        "CHAR_JUMP_1", "基础跳跃", "永久提升 11.0 跳跃高度。",
        CardCategory::Character, CardRarity::Common,
        [](GameScreen* gs){ gs->getPlayer()->applyJumpBonus(11.0f); }
    });

    // -- 稀有 (Rare) --
    m_cardPool[CardCategory::Character][CardRarity::Rare].append({
        "CHAR_SPEED_2", "肾上腺素", "永久提升 11.0 基础滑行速度。",
        CardCategory::Character, CardRarity::Rare,
        [](GameScreen* gs){ gs->getPlayer()->applySpeedBonus(11.0f); }
    });
    m_cardPool[CardCategory::Character][CardRarity::Rare].append({
        "CHAR_INVINCIBILITY_1", "能量护盾", "获得8秒的无敌护盾。",
        CardCategory::Character, CardRarity::Rare,
        [](GameScreen* gs){ gs->getPlayer()->startInvincibility(8000); }
    });

    // -- 传说 (Legendary) --
    m_cardPool[CardCategory::Character][CardRarity::Legendary].append({
        "CHAR_SPEED_3", "风之祝福", "永久提升 22.0 基础滑行速度。",
        CardCategory::Character, CardRarity::Legendary,
        [](GameScreen* gs){ gs->getPlayer()->applySpeedBonus(22.0f); }
    });


    // =================================================================
    // 得分 (Score) 相关卡池
    // =================================================================

    // -- 普通 (Common) --
    m_cardPool[CardCategory::Score][CardRarity::Common].append({
        "SCORE_MULT_1", "好运", "接下来5秒内，得分变为1.5倍。",
        CardCategory::Score, CardRarity::Common,
        [](GameScreen* gs){ gs->applyScoreMultiplier(1.5f, 5000); }
    });

    // -- 稀有 (Rare) --
    m_cardPool[CardCategory::Score][CardRarity::Rare].append({
        "SCORE_MULT_2", "幸运四叶草", "接下来5秒内，得分变为2.0倍。",
        CardCategory::Score, CardRarity::Rare,
        [](GameScreen* gs){ gs->applyScoreMultiplier(2.0f, 5000); }
    });

    // -- 传说 (Legendary) --
    m_cardPool[CardCategory::Score][CardRarity::Legendary].append({
        "SCORE_MULT_3", "财神附体", "接下来5秒内，得分变为3.0倍。",
        CardCategory::Score, CardRarity::Legendary,
        [](GameScreen* gs){ gs->applyScoreMultiplier(3.0f, 5000); }
    });


    // =================================================================
    // 雪崩 (Avalanche) 相关卡池
    // =================================================================

    // -- 普通 (Common) --
    m_cardPool[CardCategory::Avalanche][CardRarity::Common].append({
        "AVAL_SLOW_1", "冰霜陷阱", "雪崩速度减缓，持续15秒。",
        CardCategory::Avalanche, CardRarity::Common,
        [](GameScreen* gs){ if(gs->getAvalanche()) gs->getAvalanche()->applySlowDown(-0.001f, 15000); }
    });

    // -- 稀有 (Rare) --
    m_cardPool[CardCategory::Avalanche][CardRarity::Rare].append({
        "AVAL_SLOW_2", "时间沙漏", "雪崩速度大幅减缓，持续15秒。",
        CardCategory::Avalanche, CardRarity::Rare,
        [](GameScreen* gs){ if(gs->getAvalanche()) gs->getAvalanche()->applySlowDown(-0.0015f, 15000); }
    });

    // -- 传说 (Legendary) --
    m_cardPool[CardCategory::Avalanche][CardRarity::Legendary].append({
        "AVAL_PUSHBACK_1", "冲击波", "立刻将雪崩推后一段距离。",
        CardCategory::Avalanche, CardRarity::Legendary,
        [](GameScreen* gs){ if(gs->getAvalanche()) gs->getAvalanche()->pushBack(200.0f); }
    });
}
