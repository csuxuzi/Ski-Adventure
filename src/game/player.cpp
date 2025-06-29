#include "game/Player.h"
#include <QDebug>
#include <QTimer>
// 定义一些物理常量，方便调整
const qreal GRAVITY = 0.6;         // 重力加速度
const qreal JUMP_FORCE = -16.5;    // 向上的跳跃冲力 (Y轴向上为负)
const int ANIMATION_FRAME_RATE = 50; // 动画帧率，单位：毫秒 (50ms = 20 FPS)
// ... 在构造函数顶部，定义常量
const int CRASH_DURATION_MS = 5000; // 默认摔倒时间 5 秒
const qreal CRASH_TIME_REDUCTION_MS = 500; // 每次点击减少 0.5 秒
// ...
const int CRASH_LOOP_FRAME_RATE = 250; // 【新增】摔倒循环动画的帧率 (250ms)，数字越大越慢
// ...
Player::Player(QObject *parent) : GameObject(parent)
    , m_currentFrameIndex(0),// 【新增】初始化新成员
    m_crashStateTimer(new QTimer(this)),
    m_crashTimeRemaining(0),
    m_crashInitialLoopCount(0)
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

    // 【新增】加载摔倒动画 (假设有4帧, 名为 crash_1.png 到 crash_4.png)
    for (int i = 1; i <= 7; ++i) {
        QPixmap frame;
        QString path = QString(":/assets/images/player/crash_%1.png").arg(i);
        if (frame.load(path)) {
            m_crashingFrames.append(frame);
        } else {
            qWarning() << "Failed to load crash frame:" << path;
        }
    }

    // 【新增】加载站立动画 (假设有10帧, standup_1.png 到 standup_10.png)
    for (int i = 1; i <= 10; ++i) {
        QPixmap frame;
        QString path = QString(":/assets/images/player/standup_%1.png").arg(i);
        if (frame.load(path)) {
            m_standingUpFrames.append(frame);
        } else {
            qWarning() << "Failed to load standup frame:" << path;
        }
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
    // 【新增】连接摔倒状态计时器
    connect(m_crashStateTimer, &QTimer::timeout, this, &Player::handleCrashTimeout);
    // 注意：这里先不启动计时器，在状态切换时启动

}

void Player::update()
{

    // 【修改】在摔倒或站起时，不应用物理，也不切换状态
    if (currentState == Crashing || currentState == StandingUp) {
        // 如果正在摔倒或站起，我们只更新位置（通常速度为0），不处理其他逻辑
        m_position += m_velocity.toPointF();
        return; // 直接返回，不执行下面的状态切换
    }

    // --- 1. 应用重力 ---
    // 只要不在地面上，就持续施加重力
    if (!onGround) {
        m_velocity.setY(m_velocity.y() + GRAVITY);
    }


    // --- 2. 根据速度更新位置 ---
    m_position += m_velocity.toPointF();


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
    if (onGround && currentState != Crashing && currentState != StandingUp) {
        // 只有在地面上时才能跳跃
        // 【修改】在摔倒或站起时不能跳跃
        if (onGround) {
            // --- 【核心修正】立即切换到跳跃状态并开始播放动画 ---
            currentState = Jumping;
            setCurrentAnimation(Jumping);

            // 施加一个向上的瞬时速度
            m_velocity.setY(m_velocity.y() + JUMP_FORCE);
            onGround = false; // 立刻标记为离地状态

        }
    }
}


// --- 新增：根据状态设置当前应该播放哪一套动画 ---
void Player::setCurrentAnimation(PlayerState state)
{
    m_currentFrameIndex = 0; // 每次切换状态都从第一帧开始
    m_animationTimer->stop();  // 停止旧的动画
    // 【推荐修复】在启动计时器前，总是先恢复到默认的快节奏
    m_animationTimer->setInterval(ANIMATION_FRAME_RATE);
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
        // 【新增】处理摔倒状态
    case Crashing:
        // 摔倒状态，开始播放摔倒动画
        if (!m_crashingFrames.isEmpty()) {
            m_animationTimer->start(ANIMATION_FRAME_RATE);
        }
        break;
    case StandingUp:
        if (!m_standingUpFrames.isEmpty()) {
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
    int frameOffset = 0; // 用于截取动画序列
    int frameCount = 0;

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
    case StandingUp: // 【新增】
        currentFrames = &m_standingUpFrames;
        break;
    case Crashing:
        currentFrames = &m_crashingFrames;
        // 判断是播放“前5帧”，还是循环“后2帧”
        if (m_crashInitialLoopCount < 2) { // 播放前5帧
            frameOffset = 0;
            frameCount = 5;
        } else { // 循环后2帧
            frameOffset = 5;
            frameCount = 2;
        }
        break;
    }

    if (!currentFrames || currentFrames->isEmpty()) {
        return;
    }

    // 获取实际要播放的帧数
    int totalFramesInSequence = (frameCount > 0) ? frameCount : currentFrames->size();

    m_originalPixmap = (*currentFrames)[frameOffset + m_currentFrameIndex];
    setScale(0.8);

    // 2. 循环播放当前帧
    m_currentFrameIndex++;
    if (m_currentFrameIndex >= totalFramesInSequence) {
        // 如果是滑行或下落，循环播放
        if(currentState == Sliding || currentState == Falling || (currentState == Crashing && m_crashInitialLoopCount >= 2)) {
            m_currentFrameIndex = 0;
        } else if (currentState == Crashing && m_crashInitialLoopCount < 2) {
            // “前5帧”播放完了，计数+1，并从头开始再放一次
            m_crashInitialLoopCount++;
            m_currentFrameIndex = 0;
            // 【核心修复】当“前5帧”播放完两次，即将进入“后2帧”循环时
            if (m_crashInitialLoopCount == 2) {
                // 将动画计时器的间隔从快的(50ms)改为慢的(250ms)
                m_animationTimer->setInterval(CRASH_LOOP_FRAME_RATE);
            }
        } else {
            // 跳跃、站立动画播放完后，都停在最后一帧
            m_currentFrameIndex = totalFramesInSequence - 1;
            m_animationTimer->stop();

            // 【新增】站立动画播放完毕后的处理
            if (currentState == StandingUp) {
                isInvincible = false; // 解除无敌
                // 2. 恢复一个基础的滑行速度
                setVelocity(QVector2D(12.5, 0));

                // 3. 根据情况，强制切换到下一个正常状态
                if (onGround) {
                    currentState = Sliding;
                    setCurrentAnimation(Sliding);
                } else {
                    currentState = Falling;
                    setCurrentAnimation(Falling);
                }
            }
        }
    }

    // 3. 更新原始图像并应用缩放

}

void Player::crash()
{
    // 只有在非摔倒相关状态时才能触发
    if (currentState != Crashing && currentState != StandingUp) {
        currentState = Crashing;
        isInvincible = true; // 进入无敌状态

        m_crashInitialLoopCount = 0; // 重置“前5帧”的播放计数
        m_crashTimeRemaining = CRASH_DURATION_MS; // 设置总摔倒时间
        m_crashStateTimer->start(100); // 每100ms检查一次时间

        setCurrentAnimation(Crashing);
        setVelocity(QVector2D(2.5, 0)); /// 摔倒后速度清零
    }
}

void Player::reduceCrashTime()
{
    // 只有在摔倒循环阶段才能通过点击减少时间
    if (currentState == Crashing && m_crashInitialLoopCount >= 2) {
        m_crashTimeRemaining -= CRASH_TIME_REDUCTION_MS;
    }
}

void Player::handleCrashTimeout()
{
    m_crashTimeRemaining -= 100; // 减去计时器间隔
    if (m_crashTimeRemaining <= 0) {
        m_crashStateTimer->stop();
        currentState = StandingUp;
        setCurrentAnimation(StandingUp);
    }
}
