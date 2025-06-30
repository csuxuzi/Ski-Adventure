#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QMap>
#include <QSoundEffect>
#include <QElapsedTimer> // 【新增】用于精确计时
#include <QMediaPlayer> // 【新增】
#include <QList>        // 【新增】
#include <QUrl>         // 【新增】

class QMediaPlayer;
class QAudioOutput;
class QSoundEffect;
// 【新增】前向声明 QMediaPlaylist
class QMediaPlaylist;

// 【新增】定义清晰的BGM类型
enum class BgmType {
    MainMenu,
    GameScreen
};

// 【修改】将部分音效归类为 "Destructive"
// 这样我们可以对这类音效应用统一的冷却逻辑
enum class SfxType {
    ButtonClick,
    PlayerCrash,
    StoneShatter,
    HouseShatter,
    SeesawShatter
};

class AudioManager : public QObject
{
    Q_OBJECT
public:
    static AudioManager* instance();

    // 【修改】提供一个更通用的BGM播放接口
    void playBgm(BgmType type);
    void stopMusic();
    void setMusicEnabled(bool enabled);
    void setSfxEnabled(bool enabled);
    void playSoundEffect(SfxType type);

    // --- 【新增】音量控制接口 ---
    void setMusicVolume(float volume); // volume is 0.0 to 1.0
    void setSfxVolume(float volume);   // volume is 0.0 to 1.0


    // --- 【新增】获取状态和音量的接口 ---
    bool isMusicEnabled() const;
    float getMusicVolume() const;
    bool isSfxEnabled() const;
    float getSfxVolume() const;

private slots:
    // 【新增】用于处理BGM播放状态变化的槽函数
    void onBgmStatusChanged(QMediaPlayer::MediaStatus status);

private:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;

    // 【核心】这个函数只会在游戏启动时被调用一次
    void loadAllSoundEffects();
    // 【修改】修改BGM加载函数名
    void loadBgmLists();
    bool isDestructive(SfxType type) const;

    AudioManager& operator=(const AudioManager&) = delete;

    QMediaPlayer* m_musicPlayer;
    QAudioOutput* m_musicAudioOutput;
    // 【修改】使用QList<QUrl>代替QMediaPlaylist
    QList<QUrl> m_mainMenuBgmList;
    QList<QUrl> m_gameBgmList;
    // 【新增】追踪游戏BGM播放列表的当前索引
    int m_gameBgmIndex;
    BgmType m_currentBgm;




    // 【新增】使用 QMap 来存储和管理所有的音效
    QMap<SfxType, QSoundEffect*> m_soundEffects;
    // --- 【重新引入】共享音效冷却计时器 ---
    QElapsedTimer m_destructiveSfxTimer;
    // 冷却时间可以设得很短，例如100毫秒，足以避免同帧播放，又不会让玩家感觉音效丢失
    const int DESTRUCTIVE_SFX_COOLDOWN_MS = 100;

    bool m_musicEnabled;
    bool m_sfxEnabled;
    float m_sfxVolume; // 【修改】确保这行存在，之前的文件里已经有了

};

#endif // AUDIOMANAGER_H
