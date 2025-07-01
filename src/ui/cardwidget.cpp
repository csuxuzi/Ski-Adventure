#include "CardWidget.h"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QStyleOption>
#include <QPainter>
CardWidget::CardWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
    setCursor(Qt::PointingHandCursor); // 让鼠标移上去时变成小手
    updateStyle();
}

void CardWidget::setupUI()
{
    // 设置卡片的固定大小和初始样式
    setFixedSize(180, 250);
    setStyleSheet("CardWidget { background-color: #4A5D7E; border-radius: 15px; border: 3px solid #333; }");

    m_titleLabel = new QLabel("默认标题", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: white; padding: 10px;");

    m_descriptionLabel = new QLabel("默认描述", this);
    m_descriptionLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_descriptionLabel->setWordWrap(true); // 自动换行
    m_descriptionLabel->setStyleSheet("font-size: 14px; color: #E0E0E0; padding: 10px;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_titleLabel, 1);
    layout->addWidget(m_descriptionLabel, 3);
    setLayout(layout);
}



const CardData& CardWidget::getCardData() const
{
    return m_cardData;
}


void CardWidget::setCardData(const CardData &data)
{
    m_cardData = data;
    m_titleLabel->setText(m_cardData.title);
    m_descriptionLabel->setText(m_cardData.description);
    // 数据变化后，立即更新样式
    updateStyle();
}

void CardWidget::setSelected(bool selected)
{
    m_isSelected = selected;
    // 选中状态变化后，立即更新样式
    updateStyle();
}

// 核心的样式更新函数
void CardWidget::updateStyle()
{
    QString styleSheet = "CardWidget {"
                         "  background-color: #F0F0F0;"
                         "  border-radius: 10px;";

    // 根据稀有度设置边框颜色
    QString borderColor;
    switch (m_cardData.rarity) {
    case CardRarity::Common:
        borderColor = "white";
        break;
    case CardRarity::Rare:
        borderColor = "purple";
        break;
    case CardRarity::Legendary:
        borderColor = "gold";
        break;
    }
    styleSheet += QString("  border: 3px solid %1;").arg(borderColor);

    styleSheet += "}";

    // 如果被选中，额外叠加红色聚焦框
    if (m_isSelected) {
        styleSheet += "CardWidget:hover {"
                      "  border: 3px solid red;"
                      "}";
        // 覆盖普通状态的边框
        styleSheet.replace(QString("border: 3px solid %1;").arg(borderColor), "border: 3px solid red;");
    }

    this->setStyleSheet(styleSheet);
}

void CardWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void CardWidget::mousePressEvent(QMouseEvent *event)
{
    // 当鼠标点击时，发射信号
    emit clicked(this);
    QWidget::mousePressEvent(event);
}
