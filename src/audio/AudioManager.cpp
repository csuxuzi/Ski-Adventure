#include "audio/AudioManager.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include <QSoundEffect>
#include "audio/AudioManager.h"
#include <QDebug>
AudioManager* AudioManager::instance()
{
    static AudioManager instance;
    return &instance;
}

AudioManager::AudioManager(QObject *parent)
    : QObject(parent), m_musicPlayer(new QMediaPlayer(this)), m_musicAudioOutput(new QAudioOutput(this)),
    m_musicEnabled(true), m_sfxEnabled(true),
    m_gameBgmIndex(0),
    m_currentBgm(BgmType::MainMenu),
    m_sfxVolume(0.5)
{
    // 配置背景音乐播放器
    m_musicPlayer->setAudioOutput(m_musicAudioOutput);

    m_musicAudioOutput->setVolume(0.7); // 设置初始音乐音量为 70%

    // 在构造函数中，一次性加载所有音效到内存
    loadBgmLists();
    loadAllSoundEffects();
    qDebug() << "Audio Manager initialized and all sound effects pre-loaded.";
    // 连接 BGM 播放器的状态变化信号到我们的处理槽函数
    connect(m_musicPlayer, &QMediaPlayer::mediaStatusChanged, this, &AudioManager::onBgmStatusChanged);
    // 启动冷却计时器
    m_sfxTimer.start();
}

AudioManager::~AudioManager() {}

void AudioManager::loadBgmLists()
{
    m_mainMenuBgmList.append(QUrl("qrc:/assets/audio/music/main_menu.mp3"));
    m_gameBgmList.append(QUrl("qrc:/assets/audio/music/game_bgm_1.wav"));
    m_gameBgmList.append(QUrl("qrc:/assets/audio/music/game_bgm_2.wav"));
}

void AudioManager::onBgmStatusChanged(QMediaPlayer::MediaStatus status)
{
    // 检查当前歌曲是否已播放完毕
    if (status == QMediaPlayer::EndOfMedia) {
        // 如果是游戏界面的BGM列表
        if (m_currentBgm == BgmType::GameScreen && !m_gameBgmList.isEmpty()) {
            // 索引+1，如果超出范围则回到0
                m_gameBgmIndex = (m_gameBgmIndex + 1) % m_gameBgmList.size();
            // 设置播放器源为列表中的下一首歌曲并播放
                m_musicPlayer->setSource(m_gameBgmList.at(m_gameBgmIndex));
            m_musicPlayer->play();
        }
    }
}

void AudioManager::playBgm(BgmType type)
    {
    if (m_currentBgm == type && m_musicPlayer->playbackState() == QMediaPlayer::PlayingState) return;
    m_currentBgm = type;
    stopMusic();
    if (!m_musicEnabled) return;
    if (type == BgmType::MainMenu && !m_mainMenuBgmList.isEmpty()) {
            m_musicPlayer->setSource(m_mainMenuBgmList.first());
            m_musicPlayer->setLoops(QMediaPlayer::Infinite); // 主菜单单曲循环
            m_musicPlayer->play();
        } else if (type == BgmType::GameScreen && !m_gameBgmList.isEmpty()) {
            m_gameBgmIndex = 0; // 每次都从第一首开始
            m_musicPlayer->setSource(m_gameBgmList.first());
            m_musicPlayer->setLoops(1); // 只播放一次，循环靠槽函数
            m_musicPlayer->play();
        }
    }

// 预加载函数
void AudioManager::loadAllSoundEffects()
{
    // 创建并加载音效对象
    auto createAndLoadEffect = [&](SfxType type, const QString& path) {
        QSoundEffect* effect = new QSoundEffect(this);

        effect->setSource(QUrl(path));

        // 设置默认音量，循环次数等
        effect->setVolume(m_sfxVolume);
        effect->setLoopCount(1);

        // 将加载好的音效对象存入 Map，以便随时使用
        m_soundEffects[type] = effect;
    };

    createAndLoadEffect(SfxType::ButtonClick, "qrc:/assets/audio/sfx/button_click.wav");
    createAndLoadEffect(SfxType::PlayerCrash, "qrc:/assets/audio/sfx/player_crash.wav");
    createAndLoadEffect(SfxType::StoneShatter, "qrc:/assets/audio/sfx/stone_shatter.wav");
    createAndLoadEffect(SfxType::HouseShatter, "qrc:/assets/audio/sfx/house_shatter.wav");
    createAndLoadEffect(SfxType::SeesawShatter, "qrc:/assets/audio/sfx/seesaw_shatter.wav");
    createAndLoadEffect(SfxType::PlayerSlide,  "qrc:/assets/audio/sfx/player_slide.wav");
    createAndLoadEffect(SfxType::YetiRoar,       "qrc:/assets/audio/sfx/yeti_roar.wav");
    createAndLoadEffect(SfxType::PenguinChirp,   "qrc:/assets/audio/sfx/penguin_chirp.wav");
    createAndLoadEffect(SfxType::PenguinPoof,    "qrc:/assets/audio/sfx/penguin_poof.wav");
    createAndLoadEffect(SfxType::YetiPoof,       "qrc:/assets/audio/sfx/yeti_poof2.wav");
    createAndLoadEffect(SfxType::YetiBroke,       "qrc:/assets/audio/sfx/yeti_poof1.wav");
    createAndLoadEffect(SfxType::CoinGet,        "qrc:/assets/audio/sfx/coin_get.wav");
    createAndLoadEffect(SfxType::SeesawSlide,    "qrc:/assets/audio/sfx/house_slide.wav");

    if (m_soundEffects.contains(SfxType::PlayerSlide)) {
        m_soundEffects[SfxType::PlayerSlide]->setLoopCount(QSoundEffect::Infinite);
    }
}



void AudioManager::playSoundEffect(SfxType type)
{
    if (!m_sfxEnabled || !m_soundEffects.contains(type)) {
        return;
    }

    // 检查全局音效计时器是否已超过冷却时间
    if (m_sfxTimer.elapsed() > SFX_COOLDOWN_MS) { // <--- 使用新的常量和计时器
        // 时间足够长，可以播放
        m_soundEffects[type]->play();
        // 播放后，立刻重启全局计时器
        m_sfxTimer.restart();
    }
    // 如果时间不够长，则直接忽略本次播放请求，不做任何事
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
            playBgm(m_currentBgm);
        }
    }
}

void AudioManager::setSfxEnabled(bool enabled)
{
    m_sfxEnabled = enabled;
}


// --- 【在这里添加以下四个新函数】 ---

void AudioManager::setMusicVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    // 直接设置 QAudioOutput 的音量，它控制所有BGM
    m_musicAudioOutput->setVolume(volume);

    // 如果音量从0变为非0，且音乐开关是开的，就尝试播放当前BGM
    if (volume > 0 && m_musicEnabled && m_musicPlayer->playbackState() != QMediaPlayer::PlayingState) {
        playBgm(m_currentBgm);
    }
    // 如果音量降为0，则停止音乐
    else if (volume == 0.0f) {
        stopMusic();
    }
}

void AudioManager::setSfxVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    m_sfxVolume = volume; // 存储音量值

    // 遍历所有的音效，并设置它们的音量
    for (QSoundEffect* effect : qAsConst(m_soundEffects)) {
        effect->setVolume(m_sfxVolume);
    }
}

// 获取当前音乐音量
float AudioManager::getMusicVolume() const
{
    return m_musicAudioOutput->volume();
}

// 获取当前音效音量
float AudioManager::getSfxVolume() const
{
    return m_sfxVolume;
}


void AudioManager::playContinuousSound(SfxType type)
{
    if (!m_sfxEnabled || !m_soundEffects.contains(type)) {
        return;
    }
    // 只在音效没有播放时才开始播放，避免重复
    if (m_soundEffects[type]->isPlaying() == false) {
        m_soundEffects[type]->play();
    }
}

void AudioManager::stopContinuousSound(SfxType type)
{
    if (!m_sfxEnabled || !m_soundEffects.contains(type)) {
        return;
    }
    // 只在音效正在播放时才停止
    if (m_soundEffects[type]->isPlaying()) {
        m_soundEffects[type]->stop();
    }
}

void AudioManager::stopAllContinuousSounds()
{
    // 直接针对已知的、会循环播放的滑行音效进行操作
    if (m_soundEffects.contains(SfxType::PlayerSlide)) {
        m_soundEffects[SfxType::PlayerSlide]->stop();
    }
    // 如果未来有其他循环音效，也可以在这里添加停止逻辑
}
