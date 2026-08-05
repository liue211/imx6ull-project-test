#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QMediaPlayer>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QMediaPlaylist;
class QPushButton;
class QSlider;
class QVideoWidget;

/* 视频播放器:扫描 myVideo 目录,播放/暂停/下一集/音量/全屏 */
class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = nullptr);

private slots:
    void onPlayClicked();
    void onNextClicked();
    void onVolumeUp();
    void onVolumeDown();
    void onFullscreenClicked(bool checked);
    void onStateChanged(QMediaPlayer::State state);
    void onPlaylistIndexChanged(int index);
    void onDurationChanged(qint64 duration);
    void onPositionChanged(qint64 position);
    void onDurationSliderReleased();
    void onVolumeSliderReleased();
    void onItemClicked(QListWidgetItem *item);

private:
    void setupUi();
    void scanVideos();
    static QString formatTime(qint64 ms);

    QMediaPlayer *m_player = nullptr;
    QMediaPlaylist *m_playlist = nullptr;
    QVideoWidget *m_videoWidget = nullptr;
    QListWidget *m_list = nullptr;
    QSlider *m_durationSlider = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QPushButton *m_playButton = nullptr;
    QPushButton *m_nextButton = nullptr;
    QPushButton *m_volumeUpButton = nullptr;
    QPushButton *m_volumeDownButton = nullptr;
    QPushButton *m_fullscreenButton = nullptr;
    QLabel *m_timeLabel = nullptr;
    QLabel *m_durationLabel = nullptr;
    QWidget *m_controlBar = nullptr;
    QWidget *m_sidePanel = nullptr;
};

#endif // VIDEOPLAYER_H
