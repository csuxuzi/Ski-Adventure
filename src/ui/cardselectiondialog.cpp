#include "CardSelectionDialog.h"
#include "CardWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QRandomGenerator>
#include <QDebug>

CardSelectionDialog::CardSelectionDialog(QWidget *parent) : QDialog(parent)
{
    // 设置为无边框的模态对话框
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true); // 确保它弹出时，游戏主窗口不能被点击

    setupUI();
    generateRandomCards();
}

void CardSelectionDialog::setupUI()
{
    // 半透明的黑色背景遮罩
    QWidget* backgroundWidget = new QWidget(this);
    backgroundWidget->setStyleSheet("background-color: rgba(0, 0, 0, 0.7);");

    QHBoxLayout* cardLayout = new QHBoxLayout();
    cardLayout->setSpacing(30);
    cardLayout->addStretch();
    for (int i = 0; i < 3; ++i) {
        CardWidget* card = new CardWidget();
        m_cards.append(card);
        cardLayout->addWidget(card);
        connect(card, &CardWidget::clicked, this, &CardSelectionDialog::onCardClicked);
    }
    cardLayout->addStretch();

    m_submitButton = new QPushButton("确 认", this);
    m_submitButton->setCursor(Qt::PointingHandCursor);
    m_submitButton->setFixedSize(150, 50);
    m_submitButton->setStyleSheet("QPushButton { font-size: 20px; font-weight: bold; color: white; background-color: #28a745; border: none; border-radius: 10px; }"
                                  "QPushButton:hover { background-color: #218838; }");
    m_submitButton->setEnabled(false); // 初始不可点击
    connect(m_submitButton, &QPushButton::clicked, this, &CardSelectionDialog::onSubmit);

    QVBoxLayout* mainLayout = new QVBoxLayout(backgroundWidget);
    mainLayout->addStretch(2);
    mainLayout->addLayout(cardLayout);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_submitButton, 0, Qt::AlignCenter);
    mainLayout->addStretch(1);

    // 使主布局填充整个对话框
    QVBoxLayout* dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(backgroundWidget);
    setLayout(dialogLayout);
}

// 当一张卡片被点击时的逻辑
void CardSelectionDialog::onCardClicked(CardWidget* clickedWidget)
{
    // 如果之前有选中的卡片，把它恢复原状
    if (m_selectedCard) {
        m_selectedCard->setSelected(false);
    }

    // 设置新点击的卡片为选中状态
    m_selectedCard = clickedWidget;
    m_selectedCard->setSelected(true);

    // 让“确认”按钮变为可点击
    m_submitButton->setEnabled(true);
}

const CardData& CardSelectionDialog::getSelectedCardData() const
{
    if (m_selectedCard) {
        return m_selectedCard->getCardData();
    }
    // 理论上不应该到这里，因为按钮在没选择时是禁用的
    static CardData emptyData;
    return emptyData;
}

// 当点击“确认”按钮时
void CardSelectionDialog::onSubmit()
{
    if (m_selectedCard) {
        qDebug() << "选择了卡片:" << m_selectedCard->getCardData().title;
        accept(); // 关闭对话框，并返回 QDialog::Accepted
    }
}

// 随机生成三张卡片的逻辑（这是一个简单的示例）
void CardSelectionDialog::generateRandomCards()
{
    QList<CardData> allPossibleCards;
    allPossibleCards.append({CardEffectType::IncreaseSpeed, "肾上腺素", "获得爆发性的速度，持续10秒。", 5.0f});
    allPossibleCards.append({CardEffectType::DecreaseGravity, "轻如鸿毛", "身体变轻，跳得更高，下落更慢。", 0.2f});
    allPossibleCards.append({CardEffectType::IncreaseJump, "强力弹簧", "跳跃力量大幅增强。", 5.0f});
    allPossibleCards.append({CardEffectType::SlowDownAvalanche, "时间沙漏", "让身后的雪崩速度减慢。", -0.001f});
    // 您可以添加更多卡片到这个列表里

    if (allPossibleCards.size() < 3) return;

    // 随机挑选三张不重复的卡片
    for (int i = 0; i < 3; ++i) {
        int randomIndex = QRandomGenerator::global()->bounded(allPossibleCards.size());
        m_cards[i]->setCardData(allPossibleCards.takeAt(randomIndex));
        m_cards[i]->setSelected(false); // 确保都是未选中状态
    }

    m_selectedCard = nullptr;
    m_submitButton->setEnabled(false);
}
