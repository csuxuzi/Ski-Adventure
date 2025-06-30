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
    : QObject(parent), m_musicPlayer(new QMediaPlayer(this)), m_audioOutput(new QAudioOutput(this)),
      m_musicEnabled(true), m_sfxEnabled(true),
        m_gameBgmIndex(0),
        m_currentBgm(BgmType::MainMenu)
{
    // 配置背景音乐播放器
    m_musicPlayer->setAudioOutput(m_audioOutput);

    // 【核心】在构造函数中，一次性加载所有音效到内存
    loadBgmLists();
    loadAllSoundEffects();
    qDebug() << "Audio Manager initialized and all sound effects pre-loaded.";
    // 【核心修改】连接 BGM 播放器的状态变化信号到我们的处理槽函数
    connect(m_musicPlayer, &QMediaPlayer::mediaStatusChanged, this, &AudioManager::onBgmStatusChanged);
    // 【重新引入】启动冷却计时器
    m_destructiveSfxTimer.start();
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

// 【核心实现】预加载函数
void AudioManager::loadAllSoundEffects()
{
    // 这个辅助 lambda 表达式帮助我们创建并加载音效对象
    auto createAndLoadEffect = [&](SfxType type, const QString& path) {
        QSoundEffect* effect = new QSoundEffect(this);

        // 【关键】setSource 只在这里被调用一次！
        // 这会将音频文件从资源（.qrc）加载到内存中。
        effect->setSource(QUrl(path));

        // 设置默认音量，循环次数等，这些设置也会被保留
        effect->setVolume(0.8);
        effect->setLoopCount(1);

        // 将加载好的音效对象存入 Map，以便随时使用
        m_soundEffects[type] = effect;
    };

    // --- 在这里列出您所有的独立音效 ---
    createAndLoadEffect(SfxType::ButtonClick, "qrc:/assets/audio/sfx/button_click.wav");
    createAndLoadEffect(SfxType::PlayerCrash, "qrc:/assets/audio/sfx/player_crash.wav");
    createAndLoadEffect(SfxType::StoneShatter, "qrc:/assets/audio/sfx/stone_shatter.wav");
    createAndLoadEffect(SfxType::HouseShatter, "qrc:/assets/audio/sfx/house_shatter.wav");
    createAndLoadEffect(SfxType::SeesawShatter, "qrc:/assets/audio/sfx/seesaw_shatter.wav");
}


// 【新增】辅助函数的实现
bool AudioManager::isDestructive(SfxType type) const
{
    // 在这里定义哪些音效属于“破坏性”
    return type == SfxType::StoneShatter ||
           type == SfxType::HouseShatter ||
           type == SfxType::SeesawShatter;
}

// 【核心实现】高性能的播放函数
void AudioManager::playSoundEffect(SfxType type)
{
    if (!m_sfxEnabled || !m_soundEffects.contains(type)) {
        return;
    }

    // 判断请求的音效是否为需要冷却的“破坏性”音效
    if (isDestructive(type)) {
        // 如果是，则检查共享计时器是否已超过冷却时间
        if (m_destructiveSfxTimer.elapsed() > DESTRUCTIVE_SFX_COOLDOWN_MS) {
            // 时间足够长，可以播放
            m_soundEffects[type]->play();
            // 播放后，立刻重启共享计时器
            m_destructiveSfxTimer.restart();
        }
        // 如果时间不够长，则直接忽略本次播放请求，不做任何事
    } else {
        // 如果不是破坏性音效（如按钮点击、玩家摔倒），则不受影响，直接播放
        m_soundEffects[type]->play();
    }
}

// void AudioManager::playMainMenuMusic()
// {
//     if (!m_musicEnabled) return;
//     m_musicPlayer->setSource(QUrl("qrc:/assets/audio/music/main_menu.mp3"));
//     m_musicPlayer->setLoops(QMediaPlayer::Infinite);
//     m_musicPlayer->play();
// }

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
            // playMainMenuMusic();
            playBgm(m_currentBgm);
        }
    }
}

void AudioManager::setSfxEnabled(bool enabled)
{
    m_sfxEnabled = enabled;
    // 未来在这里处理音效的逻辑
    // 如果禁用音效，可以停止所有正在播放的音效
    if (!enabled) {
        for (QSoundEffect* effect : qAsConst(m_soundEffects)) {
            effect->stop();
        }
    }
}
