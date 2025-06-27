#include "audio/AudioManager.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

AudioManager* AudioManager::instance()
{
    static AudioManager instance;
    return &instance;
}

AudioManager::AudioManager(QObject *parent)
    : QObject(parent), m_musicPlayer(new QMediaPlayer(this)), m_audioOutput(new QAudioOutput(this)),
      m_musicEnabled(true), m_sfxEnabled(true)
{
    m_musicPlayer->setAudioOutput(m_audioOutput);
}

AudioManager::~AudioManager() {}

void AudioManager::playMainMenuMusic()
{
    if (!m_musicEnabled) return;
    m_musicPlayer->setSource(QUrl("qrc:/assets/audio/music/main_menu.mp3"));
    m_musicPlayer->setLoops(QMediaPlayer::Infinite);
    m_musicPlayer->play();
}

void AudioManager::stopMusic()
{
    m_musicPlayer->stop();
}

void AudioManager::setMusicEnabled(bool enabled)
{
    m_musicEnabled = enabled;
    if (!enabled) {
        stopMusic();
    } else {
        if(m_musicPlayer->playbackState() != QMediaPlayer::PlayingState) {
            playMainMenuMusic();
        }
    }
}

void AudioManager::setSfxEnabled(bool enabled)
{
    m_sfxEnabled = enabled;
    // 未来在这里处理音效的逻辑
}