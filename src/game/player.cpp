#include "game/Player.h"
#include <QDebug>
#include <QTimer>
#include "audio/AudioManager.h"
#include "fx/EffectManager.h"
// 定义一些物理常量，方便调整
const qreal GRAVITY = 0.5;         // 重力加速度
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
    m_crashInitialLoopCount(0),
    INVINCIBILITY_DURATION_MS(500)///默认无敌1.5秒
{
    // --- 1. 加载所有动画帧 ---
    // --- 【新增】初始化速度 ---
    m_baseSpeed = 12.5; // 设置一个基础速度值
    m_currentSpeed = m_baseSpeed; // 初始时，当前速度等于基础速度

    // --- 【新增】初始化坐骑和重力相关变量 ---
    m_mountFrameIndex = 0;
    m_currentMount = None;
    m_currentGravity = GRAVITY; // 初始重力使用默认值

    // --- 【新增】初始化坐骑动画计数器 ---
    m_mountAnimCounter = 0;
    // --- 【新增】初始化玩家的默认跳跃力 ---
    m_jumpForce = -16.5; // 设置一个初始值

    // 假设您的图片名为 ride_yeti.png
    if (!m_ridingYetiPixmap.load(":/assets/images/player/ride_yeti.png")) {
        qWarning() << "Failed to load yeti riding pixmap!";
    }

    // 【新增】加载“骑乘损坏雪怪”的静态图片
    if (!m_ridingBrokenYetiPixmap.load(":/assets/images/player/ride_yeti_broken.png")) {
        qWarning() << "Failed to load broken yeti riding pixmap!";
    }

    // 【新增】加载“骑乘企鹅”的静态图片
    // 假设您的图片名为 ride_penguin.png
    if (!m_ridingPenguinPixmap.load(":/assets/images/player/ride_penguin.png")) {
        qWarning() << "Failed to load penguin riding pixmap!";
    }

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

    // 【新增】加载无敌光环的图片
    if (!m_invincibleEffectPixmap.load(":/assets/images/invincible_effect.png")) {
        qWarning() << "Failed to load invincible effect pixmap!";
    }
    // 【新增】初始化光环状态
    m_invincibleEffectOpacity = 0.0; // 一开始是完全透明的
    m_isFadingIn = false;            // 一开始不是淡入状态


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


// 【新增】实现抽象出来的无敌函数
void Player::startInvincibility()
{
    isInvincible = true;
    m_isFadingIn = true; // 【修改】开启淡入开关
    // 无敌时间结束后，自动切换为淡出状态
    QTimer::singleShot(INVINCIBILITY_DURATION_MS, this, [this](){
        isInvincible = false;
        m_isFadingIn = false; // 【修改】关闭淡入开关（即开始淡出）
    });
}

// 【新增】在 player.cpp 文件中添加这个新函数
void Player::rideMount(MountType type, const QList<QPixmap>& mountFrames, qreal newSpeed, qreal newGravity)
{
    m_currentMount = type;
    m_mountAnimationFrames = mountFrames; // 复制坐骑的动画帧
    m_mountFrameIndex = 0;

    // 应用坐骑带来的新速度和新重力
    m_currentSpeed = newSpeed;
    m_currentGravity = newGravity;

    // 确保动画计时器在运行以播放坐骑动画
    if (!m_animationTimer->isActive()) {
        m_animationTimer->start(ANIMATION_FRAME_RATE);
    }
}

void Player::update()
{

    // 【新增】每帧更新光环的透明度
    const qreal FADE_SPEED = 0.05; // 定义一个统一的淡入淡出速度，方便调整
    if (m_isFadingIn) {
        // 如果是淡入状态，且还没完全显示出来，就增加透明度
        if (m_invincibleEffectOpacity < 1.0) {
            m_invincibleEffectOpacity += FADE_SPEED;
            if (m_invincibleEffectOpacity > 1.0) m_invincibleEffectOpacity = 1.0;
        }
    } else {
        // 如果是淡出状态，且还没完全消失，就减少透明度
        if (m_invincibleEffectOpacity > 0.0) {
            m_invincibleEffectOpacity -= FADE_SPEED;
            if (m_invincibleEffectOpacity < 0.0) m_invincibleEffectOpacity = 0.0;
        }
    }

    // 【修改】在摔倒或站起时，不应用物理，也不切换状态
    if (currentState == Crashing || currentState == StandingUp) {
        // 如果正在摔倒或站起，我们只更新位置（通常速度为0），不处理其他逻辑
        m_position += m_velocity.toPointF();
        return; // 直接返回，不执行下面的状态切换
    }

    // --- 1. 应用重力 ---
    // 只要不在地面上，就持续施加重力
    if (!onGround) {
        // 【修改】使用 m_currentGravity 而不是固定的 GRAVITY
        m_velocity.setY(m_velocity.y() + m_currentGravity);
    }

    // 3. 【核心】将速度矢量归一化，然后乘以当前速度大小
    //    这确保了无论方向如何，速度的总大小始终为 m_currentSpeed
    m_velocity.normalize();
    m_velocity *= m_currentSpeed;

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
            m_velocity.setY(m_velocity.y() + m_jumpForce);
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

    // --- 【新增】如果玩家正在骑乘，则优先更新坐骑的动画 ---
    // if (isMounted()) {
    //     if (!m_mountAnimationFrames.isEmpty()) {
    //         m_mountFrameIndex = (m_mountFrameIndex + 1) % m_mountAnimationFrames.size();
    //     }
    //     // 此时，玩家自身的动画可以强制为“骑乘姿势”（这里我们用滑行帧代替）
    //     if (!m_slidingFrames.isEmpty()) {
    //         m_originalPixmap = m_slidingFrames.first();
    //         setScale(0.8);
    //     }
    //     return; // 处理完坐骑动画后直接返回
    // }
    // 如果没骑坐骑，或者骑着坐骑但在空中（跳跃/下落），则正常播放玩家自己的动画
    if (!isMounted() || (isMounted() && !onGround)) {
        // 【重要】确保计时器在运行，以便播放跳跃/下落动画
        if (!m_animationTimer->isActive()) {
            m_animationTimer->start(ANIMATION_FRAME_RATE);
        }
    } else if (isMounted() && onGround) {
        // --- 【特殊规则】如果骑着坐骑并且在地面上（滑行），则定格玩家动画 ---
        // 1. 直接使用滑行状态的最后一帧作为骑乘姿势
        if (!m_slidingFrames.isEmpty()) {
            m_originalPixmap = m_slidingFrames.first(); // 用第一帧或最后一帧都可以
            setScale(0.8);
        }
        // 【核心】根据坐骑类型，把玩家的图换成对应的静态骑行姿势
        // if (m_currentMount == Yeti) {
        //     m_originalPixmap = m_ridingYetiPixmap;
        // } else if (m_currentMount == Penguin) {
        //     m_originalPixmap = m_ridingPenguinPixmap;
        // }
        // 2.【重要】停止计时器，让玩家动画不再更新
        m_animationTimer->stop();
        return; // 直接返回，不执行下面的逻辑
    }


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

// 【新增】在 player.cpp 中实现 Player 专属的 draw 函数
void Player::draw(QPainter* painter)
{
    // 如果没有骑乘坐骑，就直接调用基类（GameObject）的默认绘制方法
    if (!isMounted()) {
        GameObject::draw(painter);
        return;
    }

    // 1. 每次绘制时，计数器加1
    m_mountAnimCounter++;

    int animation_speed_divider = 5; // 默认值
    if (m_currentMount == Penguin) {
        animation_speed_divider = 8; // 让企鹅的动画慢一点
    } else if (m_currentMount == Yeti) {
        animation_speed_divider = 4; // 让雪怪的动画快一点
    }

    // 3. 只有当计数器达到我们设定的值时，才更新动画帧
    if (m_mountAnimCounter >= animation_speed_divider) {
        m_mountAnimCounter = 0; // 重置计数器
        if (!m_mountAnimationFrames.isEmpty()) {
            m_mountFrameIndex = (m_mountFrameIndex + 1) % m_mountAnimationFrames.size();
        }
    }

    // --- 如果正在骑乘，则执行特殊的组合绘制逻辑 ---
    painter->save();
    painter->translate(m_position);
    painter->rotate(m_rotation);

    // 1. 绘制坐骑的当前动画帧
    if (!m_mountAnimationFrames.isEmpty()) {
        const QPixmap& mountPixmap = m_mountAnimationFrames[m_mountFrameIndex];

        // 2. 在坐骑的“背上”绘制玩家
        //    这个偏移量需要根据您的美术资源仔细调整
        QPointF playerOffset; // 举例：玩家Y坐标在坐骑高度70%的位置
        qreal playerExtraRotation = 0.0; // 玩家的额外旋转
        qreal mountScale = 0.5;
        qreal userScale = 1.0;
        if (m_currentMount == Yeti) {
            // --- 雪怪的设置 ---
            playerOffset = QPointF(3, -mountPixmap.height() * 0.30); // 向右偏移15像素，位置更高
            playerExtraRotation = -50.0;                              // 身体前倾8度
            mountScale = 0.8; // 雪怪放大到110%
            userScale = 0.8;
        }
        else if (m_currentMount == Penguin) {
            // --- 企鹅的设置 ---
            playerOffset = QPointF(0, -mountPixmap.height() * 0.35); // 向左偏移5像素，位置更低
            playerExtraRotation = 5.0;
            mountScale = 0.8; // 企鹅缩小到80%
        }
        else if (m_currentMount == BrokenYeti) {
            // --- 企鹅的设置 ---
            playerOffset = QPointF(0, -mountPixmap.height() * 0.20); // 向左偏移5像素，位置更低
            playerExtraRotation = 5.0;
            mountScale = 0.8; // 企鹅缩小到80%
            userScale = 0.9;
        }
        painter->save(); // 保存一个干净的坐标系状态
        painter->scale(mountScale, mountScale);
        // 坐骑的锚点在脚底中心，所以绘制时向上、向左偏移
        QPointF mountDrawPos(-mountPixmap.width() / 2.0, -mountPixmap.height());
        painter->drawPixmap(mountDrawPos, mountPixmap);
        painter->restore();


        // d. 绘制玩家（应用上面定义的专属微调）
        painter->save(); // 保存坐骑的坐标系状态
        painter->translate(playerOffset);          // 应用位置偏移
        painter->scale(userScale, userScale);
        painter->rotate(playerExtraRotation);      // 应用额外旋转

        // m_pixmap 中保存着玩家当前应该显示的帧（例如“滑行”或“骑乘”姿势）
        const QPixmap& playerPixmap = m_pixmap;
        QPointF playerDrawPos = playerOffset - QPointF(playerPixmap.width() / 2.0, playerPixmap.height());
        painter->drawPixmap(playerDrawPos, playerPixmap);

        painter->restore(); // 恢复到坐骑的坐标系
    }

    painter->restore();

    // 【新增】在画完玩家之后，再在上面叠加光环特效
    if (m_invincibleEffectOpacity > 0.0) // 只有在不完全透明时才绘制
    {
        painter->save(); // 保存当前画笔状态
        painter->setOpacity(m_invincibleEffectOpacity); // 设置画笔的透明度

        // 计算光环的位置，使其中心与玩家的中心(m_position)对齐
        QPointF effectPos = m_position - QPointF(m_invincibleEffectPixmap.width() / 2.0, m_invincibleEffectPixmap.height() / 2.0);

        // 在计算好的位置画出光环
        painter->drawPixmap(effectPos, m_invincibleEffectPixmap);

        painter->restore(); // 恢复画笔状态，以免影响其他东西的绘制
    }
}



void Player::crash()
{

    if (isInvincible) {
        return;
    }

    // 只有在非摔倒相关状态时才能触发
    if (currentState == Crashing || currentState == StandingUp) {
        return; // 如果正在摔倒或站起，则直接返回，避免重复触发
    }


    // 【新增代码】处理骑乘状态下的特殊摔倒逻辑
    if (isMounted()) {
        if (m_currentMount == Penguin) {
            // 【简单、直接】不再判断坐骑类型，直接发出请求
            EffectManager::instance()->playEffect(EffectManager::EffectType::PenguinPoof, this->position());
            // 如果骑乘的是企鹅，则执行“企鹅消失”逻辑
            m_currentMount = None;                // 1. 将坐骑状态切回“无”
            m_mountAnimationFrames.clear();       // 2. 清空坐骑的动画帧
            m_currentSpeed = m_baseSpeed;         // 3. 将玩家速度恢复为基础速度
            m_currentGravity = GRAVITY;           // 4. 将重力恢复为默认值
            //Penguin->disappear();
            // 5. 给予一个短暂的无敌状态，防止立即再次碰撞摔倒
            startInvincibility(); // <-- 【修改】调用新函数
            // 6. 【核心】直接返回，不再执行后续的标准摔倒动画和流程
            return;
        }
        else if (m_currentMount == Yeti) {
            // a. 切换为 BrokenYeti 状态，并传递其静态图片作为唯一的“动画帧”
            rideMount(BrokenYeti, {m_ridingBrokenYetiPixmap}, 16.0, 0.6); // 速度和重力略微降低

            // b. 开启短暂无敌
            startInvincibility(); // <-- 【修改】调用新函数
            return; // 同样，直接返回，不执行标准摔倒
        }
        else if (m_currentMount == BrokenYeti) {
            // 【简单、直接】不再判断坐骑类型，直接发出请求
            EffectManager::instance()->playEffect(EffectManager::EffectType::YetiPoof, this->position());
            // --- 损坏的雪怪逻辑（第二条命）：彻底消失 ---
            m_currentMount = None;
            m_currentSpeed = m_baseSpeed;
            m_currentGravity = GRAVITY;
            startInvincibility(); // <-- 【修改】调用新函数
            return;
        }
    }

    // 只有在非摔倒相关状态时才能触发
    if (currentState != Crashing && currentState != StandingUp) {
        currentState = Crashing;
        isInvincible = true; // 进入无敌状态
        // --- 【核心修改】在这里同时修改核心速度和当前速度矢量 ---
        m_currentSpeed = 2.5; // 1. 把核心速度属性降下来
        setVelocity(QVector2D(m_currentSpeed, 0)); // 2. 用新的低速来设置当前的速度矢量

        m_crashInitialLoopCount = 0; // 重置“前5帧”的播放计数
        m_crashTimeRemaining = CRASH_DURATION_MS; // 设置总摔倒时间
        m_crashStateTimer->start(100); // 每100ms检查一次时间
        // 【新增】播放摔倒音效
        AudioManager::instance()->playSoundEffect(SfxType::PlayerCrash);
        setCurrentAnimation(Crashing);
        //setVelocity(QVector2D(2.5, 0)); /// 摔倒后速度清零
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
        // --- 【新增】在站起来后，恢复基础速度 ---
        m_currentSpeed = m_baseSpeed;
    }
}


// 【新增】应用速度奖励的函数
void Player::applySpeedBonus(float bonus)
{
    // 直接增加角色的基础速度
    m_baseSpeed += bonus;
    m_currentSpeed = m_baseSpeed; // 确保当前速度也立即更新
    qDebug() << "速度提升！当前基础速度为:" << m_baseSpeed;
}

// 【新增】应用跳跃奖励的函数
void Player::applyJumpBonus(float bonus)
{
    // Y轴向上为负，所以增加跳跃高度需要减去一个正值
    m_jumpForce -= bonus;
    qDebug() << "跳跃力增强！当前跳跃力为:" << m_jumpForce;
}
