#ifndef CARDSELECTIONDIALOG_H
#define CARDSELECTIONDIALOG_H

#include <QDialog>
#include "game/CardEffects.h"

class CardWidget;
class QPushButton;

class CardSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CardSelectionDialog(QWidget *parent = nullptr);
    const CardData& getSelectedCardData() const;

private slots:
    void onCardClicked(CardWidget* clickedWidget);
    void onSubmit();

private:
    void setupUI();
    void generateRandomCards();

    QList<CardWidget*> m_cards;
    CardWidget* m_selectedCard = nullptr;
    QPushButton* m_submitButton;
};

#endif // CARDSELECTIONDIALOG_H
