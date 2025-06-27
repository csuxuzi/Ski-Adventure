#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QCheckBox;
class QPushButton;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void onMusicStateChanged(int state);
    void onSfxStateChanged(int state);
    void onExitGameClicked();
};

#endif // SETTINGSDIALOG_H