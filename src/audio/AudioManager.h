#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>

class QMediaPlayer;
class QAudioOutput;

class AudioManager : public QObject
{
    Q_OBJECT
public:
    static AudioManager* instance();

    void playMainMenuMusic();
    void stopMusic();
    void setMusicEnabled(bool enabled);
    void setSfxEnabled(bool enabled);

private:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    QMediaPlayer* m_musicPlayer;
    QAudioOutput* m_audioOutput;
    bool m_musicEnabled;
    bool m_sfxEnabled;
};

#endif // AUDIOMANAGER_H