#include "CardSelectionDialog.h"
#include "CardWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QRandomGenerator>
#include <QDebug>
#include "game/CardDatabase.h"
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

void CardSelectionDialog::generateRandomCards()
{
    // 决定这次抽卡的稀有度档次
    // 80%概率普通，15%稀有，5%传说
    CardRarity rarityToDraw = CardRarity::Common;
    int roll = QRandomGenerator::global()->bounded(100);
    if (roll < 5) {
        rarityToDraw = CardRarity::Legendary;
    } else if (roll < 20) {
        rarityToDraw = CardRarity::Rare;
    }

    // 从三个类别中，各抽取一张指定稀有度的卡片
    m_cards[0]->setCardData(CardDatabase::instance().drawCard(CardCategory::Character, rarityToDraw));
    m_cards[1]->setCardData(CardDatabase::instance().drawCard(CardCategory::Score, rarityToDraw));
    m_cards[2]->setCardData(CardDatabase::instance().drawCard(CardCategory::Avalanche, rarityToDraw));

    // 重置UI状态
    for (CardWidget* card : m_cards) {
        card->setSelected(false);
    }
    m_selectedCard = nullptr;
    m_submitButton->setEnabled(false);
}
