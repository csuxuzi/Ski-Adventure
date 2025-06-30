#include "CardWidget.h"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QStyleOption>
#include <QPainter>
CardWidget::CardWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
    setCursor(Qt::PointingHandCursor); // 让鼠标移上去时变成小手
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

void CardWidget::setCardData(const CardData &data)
{
    m_cardData = data;
    m_titleLabel->setText(data.title);
    m_descriptionLabel->setText(data.description);
}

const CardData& CardWidget::getCardData() const
{
    return m_cardData;
}

// 根据是否被选中，改变边框颜色来高亮
void CardWidget::setSelected(bool selected)
{
    m_isSelected = selected;
    if (m_isSelected) {
        setStyleSheet("CardWidget { background-color: #5A7D9E; border-radius: 15px; border: 4px solid #FFD700; }");
    } else {
        setStyleSheet("CardWidget { background-color: #4A5D7E; border-radius: 15px; border: 3px solid #333; }");
    }
}

// --- 【在这里添加下面这个新函数】 ---
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
