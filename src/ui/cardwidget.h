#ifndef CARDWIDGET_H
#define CARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include "game/CardEffects.h"

class CardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CardWidget(QWidget *parent = nullptr);

    void setCardData(const CardData& data);
    const CardData& getCardData() const;
    void setSelected(bool selected);
    void updateStyle();
signals:
    void clicked(CardWidget* widget); // 当被点击时，发射一个信号告诉父窗口

protected:
    void mousePressEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();

    QLabel* m_titleLabel;
    QLabel* m_descriptionLabel;
    CardData m_cardData;
    bool m_isSelected = false;

};

#endif // CARDWIDGET_H
