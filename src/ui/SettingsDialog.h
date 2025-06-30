#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class ImageButton;
class QSlider;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    // 重写showEvent，确保每次打开时都刷新状态
    void showEvent(QShowEvent* event) override;

private slots:
    void onMusicIconClicked();
    void onSfxIconClicked();
    void onMusicVolumeChanged(int value);
    void onSfxVolumeChanged(int value);

private:
    void setupUI();
    void updateMusicIcon();
    void updateSfxIcon();

    ImageButton* m_musicIcon;
    ImageButton* m_sfxIcon;
    QSlider* m_musicSlider;
    QSlider* m_sfxSlider;
};

#endif // SETTINGSDIALOG_H
