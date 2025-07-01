#include "fx/EffectManager.h"
#include <QPainter>
#include <QTimer>
#include <QDebug>


// 特效动画的总时长（毫秒）
const int EFFECT_DURATION_MS = 400;
const int TEXT_EFFECT_DURATION_MS = 800; // 文字持续时间稍长
const qreal TEXT_EFFECT_FLOAT_SPEED = 0.8;  // 文字上浮速度

FloatingTextEffect::FloatingTextEffect(const QString& text, const QPointF& position, int duration, const QColor& color, QObject* parent)
    : QObject(parent), m_text("+" + text), m_position(position), m_opacity(1.0), m_lifeLeft(duration), m_color(color)
{}

void FloatingTextEffect::update()
{
    if (m_lifeLeft <= 0) return;
    m_lifeLeft -= 16;
    m_position.ry() -= TEXT_EFFECT_FLOAT_SPEED; // 每帧向上移动一点
    if (m_lifeLeft < TEXT_EFFECT_DURATION_MS / 2) { // 在后半段生命周期开始淡出
        m_opacity = static_cast<qreal>(m_lifeLeft * 2) / TEXT_EFFECT_DURATION_MS;
    }
}

void FloatingTextEffect::draw(QPainter* painter)
{
    if (isFinished()) return;
    painter->save();
    QPen pen(m_color);
    painter->setPen(pen);
    QFont font = painter->font();
    font.setBold(true);
    font.setPixelSize(24);
    painter->setFont(font);
    painter->setOpacity(m_opacity);
    painter->drawText(m_position, m_text);
    painter->restore();
}

bool FloatingTextEffect::isFinished() const
{
    return m_lifeLeft <= 0;
}



VisualEffect::VisualEffect(const QPixmap &pixmap, const QPointF &position, int duration, QObject *parent)
    : QObject(parent),
    m_pixmap(pixmap),
    m_position(position),
    m_opacity(1.0),
    m_lifeLeft(duration),
    m_totalLife(duration)
{
}

void VisualEffect::update()
{
    if (m_lifeLeft <= 0) return;

    // 每帧减少16毫秒
    m_lifeLeft -= 16;

    // 根据剩余生命周期计算透明度，实现淡出效果
    if (m_totalLife > 0) {
        m_opacity = static_cast<qreal>(m_lifeLeft) / m_totalLife;
    }

    if (m_opacity < 0) {
        m_opacity = 0;
    }
}

void VisualEffect::draw(QPainter* painter)
{
    if (isFinished()) return;

    painter->save();
    painter->setOpacity(m_opacity);

    // 以特效的中心点为准进行绘制
    QPointF drawPos = m_position - QPointF(m_pixmap.width() / 2.0, m_pixmap.height() / 2.0);
    painter->drawPixmap(drawPos, m_pixmap);

    painter->restore();
}

bool VisualEffect::isFinished() const
{
    return m_lifeLeft <= 0;
}


EffectManager* EffectManager::instance()
{
    // 创建一个静态的单例实例
    static EffectManager instance;
    return &instance;
}

EffectManager::EffectManager(QObject *parent) : QObject(parent)
{
    // 在构造函数中初始化文字特效列表
    m_textEffects.clear();

    // 在构造时，加载所有特效图片并存入 Map
    QPixmap penguinPoof, yetiPoof;
    if (penguinPoof.load(":/assets/images/penguin_poof.png")) {
        m_effectPixmaps[EffectType::PenguinPoof] = penguinPoof;
    } else {
        qWarning() << "Failed to load penguin poof effect image!";
    }

    if (yetiPoof.load(":/assets/images/yeti_poof.png")) {
        m_effectPixmaps[EffectType::YetiPoof] = yetiPoof;
    } else {
        qWarning() << "Failed to load yeti poof effect image!";
    }
}

// 播放浮动分数特效的实现
void EffectManager::playFloatingScoreEffect(int score, const QPointF& position)
{
    QString scoreText = QString::number(score);
    FloatingTextEffect* effect = new FloatingTextEffect(scoreText, position, TEXT_EFFECT_DURATION_MS, QColor(255, 215, 0), this);
    m_textEffects.append(effect);
}

void EffectManager::playEffect(EffectType type, const QPointF& position)
{
    if (!m_effectPixmaps.contains(type) || m_effectPixmaps[type].isNull()) {
        qWarning() << "Attempted to play an effect of a type that was not loaded!";
        return;
    }

    const QPixmap& pixmapToPlay = m_effectPixmaps[type];

    // 创建一个新的视觉特效对象，并添加到管理列表中
    VisualEffect* effect = new VisualEffect(pixmapToPlay, position, EFFECT_DURATION_MS, this);
    m_effects.append(effect);
}

void EffectManager::update()
{
    // 更新所有正在播放的特效
    for (VisualEffect* effect : m_effects) {
        effect->update();
    }

    // 从列表中移除所有已经播放完毕的特效
    m_effects.erase(std::remove_if(m_effects.begin(), m_effects.end(),
       [](VisualEffect* effect) {
           if (effect->isFinished()) {
               delete effect; // 删除对象，防止内存泄漏
               return true;
           }
           return false;
       }), m_effects.end());

    // 更新所有文字特效
    for(auto* effect : m_textEffects) { effect->update(); }
    m_textEffects.erase(std::remove_if(m_textEffects.begin(), m_textEffects.end(),
       [](FloatingTextEffect* effect){
           if (effect->isFinished()) {
               delete effect;
               return true;
           }
           return false;
       }), m_textEffects.end());
}

void EffectManager::draw(QPainter* painter)
{
    // 这个函数现在只负责绘制图片特效
    for (VisualEffect* effect : m_effects) {
        effect->draw(painter);
    }
}

// 一个专门绘制文字特效的函数
void EffectManager::drawTextEffects(QPainter *painter)
{
    // 这个函数只负责绘制文字特效
    for(auto* effect : m_textEffects) {
        effect->draw(painter);
    }
}
