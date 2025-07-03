#ifndef DEBUGINFODIALOG_H
#define DEBUGINFODIALOG_H

#include <QDialog>

class QLabel;
class Player;

class DebugInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DebugInfoDialog(Player* player, qreal currentScoreMultiplier, QWidget *parent = nullptr);

private:
    void setupUI();
    void populateData();

    Player* m_player;
    qreal m_currentScoreMultiplier;

    // UI Elements
    QLabel* m_mountInfoLabel;
    QLabel* m_gravityLabel;
    QLabel* m_jumpForceLabel;
    QLabel* m_scoreMultiplierLabel;
    QLabel* m_brokenYetiMultiplierLabel;
};

#endif // DEBUGINFODIALOG_H
