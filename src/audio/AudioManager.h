#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QMap>
#include <QSoundEffect>
#include <QElapsedTimer>
#include <QMediaPlayer>
#include <QList>
#include <QUrl>

class QMediaPlayer;
class QAudioOutput;
class QSoundEffect;
// 前向声明 QMediaPlaylist
class QMediaPlaylist;

// 定义清晰的BGM类型
enum class BgmType {
    MainMenu,
    GameScreen
};

enum class SfxType {
    ButtonClick,
    PlayerCrash,
    StoneShatter,
    HouseShatter,
    SeesawShatter,
    PlayerSlide,
    SeesawSlide,
    YetiRoar,
    PenguinChirp,
    PenguinPoof,
    YetiPoof,
    CoinGet,
    YetiBroke
};

class AudioManager : public QObject
{
    Q_OBJECT
public:
    static AudioManager* instance();

    void playBgm(BgmType type);
    void stopMusic();
    void setMusicEnabled(bool enabled);
    void setSfxEnabled(bool enabled);
    void playSoundEffect(SfxType type);

    // 音量控制接口
    void setMusicVolume(float volume); // volume is 0.0 to 1.0
    void setSfxVolume(float volume);   // volume is 0.0 to 1.0

    void playContinuousSound(SfxType type);
    void stopContinuousSound(SfxType type);
    void stopAllContinuousSounds();
    // 获取状态和音量的接口
    bool isMusicEnabled() const;
    float getMusicVolume() const;
    bool isSfxEnabled() const;
    float getSfxVolume() const;

private slots:
    // 用于处理BGM播放状态变化的槽函数
    void onBgmStatusChanged(QMediaPlayer::MediaStatus status);

private:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;

    // 这个函数只会在游戏启动时被调用一次
    void loadAllSoundEffects();
    void loadBgmLists();

    AudioManager& operator=(const AudioManager&) = delete;

    QMediaPlayer* m_musicPlayer;
    QAudioOutput* m_musicAudioOutput;
    QList<QUrl> m_mainMenuBgmList;
    QList<QUrl> m_gameBgmList;
    int m_gameBgmIndex;
    BgmType m_currentBgm;




    // 使用 QMap 来存储和管理所有的音效
    QMap<SfxType, QSoundEffect*> m_soundEffects;
    // 共享音效冷却计时器
    QElapsedTimer m_sfxTimer;
    // 冷却时间可以设得很短，例如100毫秒，足以避免同帧播放，又不会让玩家感觉音效丢失
    const int SFX_COOLDOWN_MS = 100;

    bool m_musicEnabled;
    bool m_sfxEnabled;
    float m_sfxVolume; // 确保这行存在，之前的文件里已经有了

};

#endif // AUDIOMANAGER_H
