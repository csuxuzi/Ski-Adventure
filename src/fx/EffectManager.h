#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include <QObject>
#include <QList>
#include <QPointF>
#include <QPixmap>
#include <QMap>
class QPainter;
class QTimer;


// 一个专门用于浮动文字的独立类
class FloatingTextEffect : public QObject
{
    Q_OBJECT
public:
    FloatingTextEffect(const QString& text, const QPointF& position, int duration, const QColor& color, QObject* parent = nullptr);
    void update();
    void draw(QPainter* painter);
    bool isFinished() const;

private:
    QString m_text;
    QPointF m_position;
    qreal m_opacity;
    int m_lifeLeft;
    QColor m_color;
};

// 一个独立的动画效果对象
class VisualEffect : public QObject
{
    Q_OBJECT
public:


    VisualEffect(const QPixmap &pixmap, const QPointF &position, int duration, QObject *parent = nullptr);
    void update();
    void draw(QPainter* painter);
    bool isFinished() const;

private:
    QPixmap m_pixmap;
    QPointF m_position;
    qreal m_opacity;
    int m_lifeLeft; // 剩余生命周期（毫秒）
    int m_totalLife;
};


// 全局特效管理器
class EffectManager : public QObject
{
    Q_OBJECT
public:
    // 定义一个清晰的特效类型枚举
    enum class EffectType {
        PenguinPoof,
        YetiPoof
        // 未来可以继续添加其他特效类型
    };

    // 播放浮动分数特效的接口
    void playFloatingScoreEffect(int score, const QPointF& position);


    static EffectManager* instance();
    // 核心接口现在接收一个 EffectType
    void playEffect(EffectType type, const QPointF& position);
    void update();
    void draw(QPainter* painter);         // 用于绘制世界坐标的图片特效
    void drawTextEffects(QPainter* painter); // 用于绘制UI坐标的文字特效

private:
    explicit EffectManager(QObject *parent = nullptr);
    QList<VisualEffect*> m_effects; // 管理所有正在播放的特效
    // 使用 QMap 来存储不同类型的特效图片
    QMap<EffectType, QPixmap> m_effectPixmaps;

    // 一个列表来管理所有浮动的文字
    QList<FloatingTextEffect*> m_textEffects;
};

#endif // EFFECTMANAGER_H
