#include "DebugInfoDialog.h"
#include "game/Player.h"
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>

DebugInfoDialog::DebugInfoDialog(Player* player, qreal currentScoreMultiplier, QWidget *parent)
    : QDialog(parent), m_player(player), m_currentScoreMultiplier(currentScoreMultiplier)
{
    setupUI();
    populateData();
}

void DebugInfoDialog::setupUI()
{
    setWindowTitle("实时调试信息");
    setFixedSize(400, 400);

    // 设置暗黑主题样式表
    setStyleSheet(R"(
        QDialog {
            background-color: #2c3e50;
            border: 1px solid #34495e;
            font-family: "Microsoft YaHei", sans-serif;
        }
        QLabel {
            color: #ecf0f1;
            font-size: 14px;
            padding: 4px;
        }
        QLabel#title {
            font-size: 16px;
            font-weight: bold;
            color: #1abc9c;
            padding-bottom: 10px;
            border-bottom: 1px solid #34495e;
            margin-bottom: 5px;
        }
        QLabel.keyLabel {
            color: #95a5a6;
            font-weight: bold;
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // --- 角色状态部分 ---
    QLabel* charTitle = new QLabel("角色状态", this);
    charTitle->setObjectName("title");
    mainLayout->addWidget(charTitle);

    QFormLayout* charLayout = new QFormLayout();
    charLayout->setSpacing(10);
    charLayout->setLabelAlignment(Qt::AlignRight);

    m_mountInfoLabel = new QLabel(this);
    m_gravityLabel = new QLabel(this);
    m_jumpForceLabel = new QLabel(this);
    m_brokenYetiMultiplierLabel = new QLabel(this);
    QLabel *key1 = new QLabel("当前坐骑:", this); key1->setProperty("class", "keyLabel");
    QLabel *key2 = new QLabel("重力加速度:", this); key2->setProperty("class", "keyLabel");
    QLabel *key3 = new QLabel("弹跳力:", this); key3->setProperty("class", "keyLabel");

    charLayout->addRow(key1, m_mountInfoLabel);
    charLayout->addRow(key2, m_gravityLabel);
    charLayout->addRow(key3, m_jumpForceLabel);
    mainLayout->addLayout(charLayout);
    mainLayout->addSpacing(15);

    // --- 游戏参数部分 ---
    QLabel* gameTitle = new QLabel("游戏参数", this);
    gameTitle->setObjectName("title");
    mainLayout->addWidget(gameTitle);

    QFormLayout* gameLayout = new QFormLayout();
    gameLayout->setSpacing(10);
    gameLayout->setLabelAlignment(Qt::AlignRight);

    m_scoreMultiplierLabel = new QLabel(this);
    QLabel *key4 = new QLabel("当前分数倍率:", this); key4->setProperty("class", "keyLabel");
    QLabel *key5 = new QLabel("企鹅速度倍率:", this); key5->setProperty("class", "keyLabel");
    QLabel *key6 = new QLabel("雪怪速度倍率:", this); key6->setProperty("class", "keyLabel");
    QLabel *key7 = new QLabel(tr("破碎雪怪倍率:"), this); key7->setProperty("class", "keyLabel");

    gameLayout->addRow(key4, m_scoreMultiplierLabel);
    gameLayout->addRow(key5, new QLabel("x1.2", this));
    gameLayout->addRow(key6, new QLabel("x1.6", this));
    gameLayout->addRow(key7, m_brokenYetiMultiplierLabel);
    mainLayout->addLayout(gameLayout);

    mainLayout->addStretch();
    setLayout(mainLayout);
}

void DebugInfoDialog::populateData()
{
    if (m_player) {
        QString mountInfoText = "无";
        if (m_player->isMounted()) {
            switch(m_player->currentMountType()) {
            case Player::Penguin: mountInfoText = "企鹅"; break;
            case Player::Yeti: mountInfoText = "雪怪"; break;
            case Player::BrokenYeti: mountInfoText = "损坏的雪怪"; break;
            default: break;
            }
        }
        m_mountInfoLabel->setText(mountInfoText);
        m_gravityLabel->setText(QString::number(m_player->currentGravity(), 'f', 2));
        m_jumpForceLabel->setText(QString::number(m_player->jumpForce(), 'f', 2));
    }

    m_scoreMultiplierLabel->setText(QString("x%1").arg(QString::number(m_currentScoreMultiplier, 'f', 1)));
    m_brokenYetiMultiplierLabel->setText(QString("x%1").arg(Player::PLAYER_SPEED_MULTIPLIER_ON_BROKEN_YETI));
}
