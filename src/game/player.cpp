#include "game/Player.h"
#include <QDebug>
#include <QTimer>
// 定义一些物理常量，方便调整
const qreal GRAVITY = 0.6;         // 重力加速度
const qreal JUMP_FORCE = -17.5;    // 向上的跳跃冲力 (Y轴向上为负)
const int ANIMATION_FRAME_RATE = 50; // 动画帧率，单位：毫秒 (50ms = 20 FPS)
Player::Player(QObject *parent) : GameObject(parent), m_currentFrameIndex(0)
{
    // --- 1. 加载所有动画帧 ---

    // 加载滑行动画（假设只有一张图片）
    QPixmap slidePixmap;
    if (slidePixmap.load(":/assets/images/player.png")) {
        m_slidingFrames.append(slidePixmap);
    } else {
        qWarning("Failed to load sliding image!");
    }

    // 加载跳跃动画 (假设有10帧, 名为 jump_1.png 到 jump_10.png)
    for (int i = 7; i <= 10; ++i) {
        QPixmap frame;
        QString path = QString(":/assets/images/player/jump_%1.png").arg(i);
        if (frame.load(path)) {
            m_jumpingFrames.append(frame);
        } else {
            qWarning() << "Failed to load jump frame:" << path;
        }
    }

    // 加载下落动画 (您可以创建单独的下落帧，这里我们暂时复用跳跃的最后一帧作为示例)
    if (!m_jumpingFrames.isEmpty()) {
        m_fallingFrames.append(m_jumpingFrames.last());
    }


    // --- 2. 初始化并启动动画系统 ---

    // 初始显示滑行状态的第一帧
    if (!m_slidingFrames.isEmpty()) {
        m_originalPixmap = m_slidingFrames.first();
        setScale(0.8); // 初始缩放
    }

    // 创建动画计时器
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &Player::updateAnimation);
    // 注意：这里先不启动计时器，在状态切换时启动






    // // 1. 【修正】将图像加载到 m_originalPixmap 中
    // if (!m_originalPixmap.load(":/assets/images/player.png")) {
    //     qWarning("Failed to load player image!");
    //     // 如果加载失败，创建一个占位符
    //     m_originalPixmap = QPixmap(50, 50);
    //     m_originalPixmap.fill(Qt::blue);
    // }

    // // 2. 现在调用 setScale，它会根据 m_originalPixmap 正确地生成缩放后的 m_pixmap
    // setScale(0.8); // 您可以按需调整此比例，例如 0.8 就是 80%

    // 【动画集成点】
    // 在这里加载您的动画帧
    // for (int i = 0; i < 10; ++i) {
    //     m_jumpFrames.append(QPixmap(QString(":/assets/images/jump/frame_%1.png").arg(i)));
    // }                      m_pixmap.width(), m_pixmap.height());
}

void Player::update()
{
    // --- 1. 应用重力 ---
    // 只要不在地面上，就持续施加重力
    if (!onGround) {
        m_velocity.setY(m_velocity.y() + GRAVITY);
    }

    // --- 2. 根据速度更新位置 ---
    m_position += m_velocity.toPointF();

    // // --- 状态与动画切换 ---
    // PlayerState previousState = currentState; // 保存之前的状态

    // // --- 3. 更新动画状态 (基于物理状态) ---
    // if (!onGround) {
    //     if (m_velocity.y() < 0) {
    //         currentState = Jumping;
    //         // 【动画集成点】在这里开始播放“起跳/上升”的动画序列
    //         // 例如: m_pixmap = m_jumpFrames[m_currentFrameIndex];
    //     } else {
    //         currentState = Falling;
    //         // 【动画集成点】在这里开始播放“下落”的动画序列
    //         // 例如: m_pixmap = m_fallFrames[m_currentFrameIndex];
    //     }
    // } else {
    //     currentState = Sliding;
    //     // 【动画集成点】在这里切换回“滑行”的单帧或动画序列
    //     // 例如: setScale(0.8); // 恢复原始缩放的图像
    // }

    // // 如果状态发生了变化，就更新当前的动画序列
    // if (currentState != previousState) {
    //     setCurrentAnimation(currentState);
    // }
    // --- 【修正】状态切换逻辑简化 ---
    // 保存之前的状态
    PlayerState previousState = currentState;

    // 只在空中时判断是上升还是下落
    if (!onGround) {
        if (m_velocity.y() < 0) {
            currentState = Jumping;
        } else {
            // 只有在非跳跃状态时才切换到下落，避免覆盖跳跃动画
            if(currentState != Jumping){
                currentState = Falling;
            }
        }
    } else {
        currentState = Sliding;
    }

    // 如果状态发生了变化，就更新当前的动画序列
    if (currentState != previousState) {
        setCurrentAnimation(currentState);
    }
}

void Player::jump()
{
    // 只有在地面上时才能跳跃
    if (onGround) {
        // --- 【核心修正】立即切换到跳跃状态并开始播放动画 ---
        currentState = Jumping;
        setCurrentAnimation(Jumping);

        // 施加一个向上的瞬时速度
        m_velocity.setY(m_velocity.y() + JUMP_FORCE);
        onGround = false; // 立刻标记为离地状态

    }
}


// --- 新增：根据状态设置当前应该播放哪一套动画 ---
void Player::setCurrentAnimation(PlayerState state)
{
    m_currentFrameIndex = 0; // 每次切换状态都从第一帧开始
    m_animationTimer->stop();  // 停止旧的动画

    switch (state) {
    case Sliding:
        // 滑行状态不播动画，直接显示第一帧
        if (!m_slidingFrames.isEmpty()) {
            m_originalPixmap = m_slidingFrames.first();
            setScale(0.8);
        }
        break;
    case Jumping:
        // 跳跃状态，开始播放跳跃动画
        if (!m_jumpingFrames.isEmpty()) {
            m_animationTimer->start(ANIMATION_FRAME_RATE);
        }
        break;
    case Falling:
        // 下落状态，开始播放下落动画
        if (!m_fallingFrames.isEmpty()) {
            m_animationTimer->start(ANIMATION_FRAME_RATE);
        }
        break;
    }
    // 立即更新一次动画，避免状态切换时的闪烁
    updateAnimation();
}


// --- 新增：更新动画帧的槽函数 ---
void Player::updateAnimation()
{
    const QList<QPixmap>* currentFrames = nullptr;

    // 1. 决定使用哪个动画帧列表
    switch (currentState) {
    case Sliding:
        currentFrames = &m_slidingFrames;
        break;
    case Jumping:
        currentFrames = &m_jumpingFrames;
        break;
    case Falling:
        currentFrames = &m_fallingFrames;
        break;
    }

    if (!currentFrames || currentFrames->isEmpty()) {
        return;
    }

    // 2. 循环播放当前帧
    m_currentFrameIndex++;
    if (m_currentFrameIndex >= currentFrames->size()) {
        // 如果是滑行或下落，循环播放
        if(currentState == Sliding || currentState == Falling) {
            m_currentFrameIndex = 0;
        } else {
            // 如果是跳跃动画，播放完后停在最后一帧
            m_currentFrameIndex = currentFrames->size() - 1;
            m_animationTimer->stop();
        }
    }

    // 3. 更新原始图像并应用缩放
    m_originalPixmap = (*currentFrames)[m_currentFrameIndex];
    setScale(0.8);
}
