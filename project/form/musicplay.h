#ifndef MUSICPLAY_H
#define MUSICPLAY_H

#include <QWidget>
#include <QMediaPlayer>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QMediaPlaylist;
class QPushButton;
class QSlider;

/* 音频播放器:扫描 myMusic 目录,播放/暂停/上下曲/进度条 */
class MusicPlay : public QWidget
{
    Q_OBJECT

public:
    explicit MusicPlay(QWidget *parent = nullptr);

private slots:
    void onPlayClicked();
    void onNextClicked();
    void onPrevClicked();
    void onStateChanged(QMediaPlayer::State state);
    void onPlaylistIndexChanged(int index);
    void onDurationChanged(qint64 duration);
    void onPositionChanged(qint64 position);
    void onSliderReleased();
    void onItemClicked(QListWidgetItem *item);

private:
    void setupUi();
    void scanSongs();
    static QString formatTime(qint64 ms);

    QMediaPlayer *m_player = nullptr;
    QMediaPlaylist *m_playlist = nullptr;
    QListWidget *m_list = nullptr;
    QSlider *m_durationSlider = nullptr;
    QPushButton *m_prevButton = nullptr;
    QPushButton *m_playButton = nullptr;
    QPushButton *m_nextButton = nullptr;
    QPushButton *m_favoriteButton = nullptr;
    QPushButton *m_modeButton = nullptr;
    QPushButton *m_menuButton = nullptr;
    QPushButton *m_volumeButton = nullptr;
    QLabel *m_cdLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    QLabel *m_durationLabel = nullptr;
};

#endif // MUSICPLAY_H
