#include "videoplayer.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QMediaPlaylist>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QVideoWidget>

VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
{
    QFile qss(QStringLiteral(":/style/video_style.qss"));
    if (qss.exists()) {
        qss.open(QFile::ReadOnly);
        setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }

    setupUi();

    m_player = new QMediaPlayer(this);
    m_playlist = new QMediaPlaylist(this);
    m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
    m_player->setPlaylist(m_playlist);
    m_player->setVideoOutput(m_videoWidget);
    m_player->setVolume(50);

    connect(m_playButton, &QPushButton::clicked, this, &VideoPlayer::onPlayClicked);
    connect(m_nextButton, &QPushButton::clicked, this, &VideoPlayer::onNextClicked);
    connect(m_volumeUpButton, &QPushButton::clicked, this, &VideoPlayer::onVolumeUp);
    connect(m_volumeDownButton, &QPushButton::clicked, this, &VideoPlayer::onVolumeDown);
    connect(m_fullscreenButton, &QPushButton::clicked, this, &VideoPlayer::onFullscreenClicked);
    connect(m_player, &QMediaPlayer::stateChanged, this, &VideoPlayer::onStateChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &VideoPlayer::onDurationChanged);
    connect(m_player, &QMediaPlayer::positionChanged, this, &VideoPlayer::onPositionChanged);
    connect(m_playlist, &QMediaPlaylist::currentIndexChanged,
            this, &VideoPlayer::onPlaylistIndexChanged);
    connect(m_list, &QListWidget::itemClicked, this, &VideoPlayer::onItemClicked);
    connect(m_durationSlider, &QSlider::sliderReleased,
            this, &VideoPlayer::onDurationSliderReleased);
    connect(m_volumeSlider, &QSlider::sliderReleased,
            this, &VideoPlayer::onVolumeSliderReleased);

    scanVideos();
}

void VideoPlayer::setupUi()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    /* 左侧播放区 */
    auto *videoPanel = new QWidget(this);
    videoPanel->setObjectName(QStringLiteral("vWidget0"));
    auto *videoLayout = new QVBoxLayout(videoPanel);
    videoLayout->setContentsMargins(0, 0, 0, 0);

    m_videoWidget = new QVideoWidget(videoPanel);
    videoLayout->addWidget(m_videoWidget, 1);

    m_controlBar = new QWidget(videoPanel);
    m_controlBar->setObjectName(QStringLiteral("vWidget1"));
    auto *barLayout = new QVBoxLayout(m_controlBar);
    barLayout->setContentsMargins(8, 4, 8, 4);

    m_durationSlider = new QSlider(Qt::Horizontal, m_controlBar);
    m_durationSlider->setObjectName(QStringLiteral("durationSlider"));
    barLayout->addWidget(m_durationSlider);

    auto *controlRow = new QHBoxLayout();
    m_playButton = new QPushButton(m_controlBar);
    m_nextButton = new QPushButton(m_controlBar);
    m_volumeDownButton = new QPushButton(m_controlBar);
    m_volumeSlider = new QSlider(Qt::Horizontal, m_controlBar);
    m_volumeUpButton = new QPushButton(m_controlBar);
    m_fullscreenButton = new QPushButton(m_controlBar);

    m_playButton->setObjectName(QStringLiteral("btn_play"));
    m_nextButton->setObjectName(QStringLiteral("btn_next"));
    m_volumeDownButton->setObjectName(QStringLiteral("btn_volumedown"));
    m_volumeUpButton->setObjectName(QStringLiteral("btn_volumeup"));
    m_fullscreenButton->setObjectName(QStringLiteral("btn_screen"));
    m_volumeSlider->setObjectName(QStringLiteral("volumeSlider"));
    m_playButton->setCheckable(true);
    m_fullscreenButton->setCheckable(true);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(50);
    m_volumeSlider->setFixedWidth(120);

    m_timeLabel = new QLabel(QStringLiteral("00:00"), m_controlBar);
    m_durationLabel = new QLabel(QStringLiteral("/00:00"), m_controlBar);

    m_playButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_play.png")));
    m_nextButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_next.png")));
    m_volumeDownButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_volume_down.png")));
    m_volumeUpButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_volume_up.png")));
    m_fullscreenButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_fullscreen.png")));

    controlRow->addWidget(m_playButton);
    controlRow->addWidget(m_nextButton);
    controlRow->addWidget(m_volumeDownButton);
    controlRow->addWidget(m_volumeSlider);
    controlRow->addWidget(m_volumeUpButton);
    controlRow->addStretch();
    controlRow->addWidget(m_timeLabel);
    controlRow->addWidget(m_durationLabel);
    controlRow->addWidget(m_fullscreenButton);
    barLayout->addLayout(controlRow);
    videoLayout->addWidget(m_controlBar);

    root->addWidget(videoPanel, 1);

    /* 右侧视频列表 */
    m_sidePanel = new QWidget(this);
    m_sidePanel->setObjectName(QStringLiteral("card"));
    auto *sideLayout = new QVBoxLayout(m_sidePanel);
    sideLayout->setContentsMargins(8, 8, 8, 8);
    m_list = new QListWidget(m_sidePanel);
    m_list->setObjectName(QStringLiteral("listWidget"));
    m_list->setMaximumWidth(260);
    sideLayout->addWidget(m_list);
    root->addWidget(m_sidePanel);
}

QString VideoPlayer::formatTime(qint64 ms)
{
    const int totalSeconds = static_cast<int>(ms / 1000);
    return QStringLiteral("%1:%2")
        .arg(totalSeconds / 60, 2, 10, QLatin1Char('0'))
        .arg(totalSeconds % 60, 2, 10, QLatin1Char('0'));
}

void VideoPlayer::scanVideos()
{
    const QDir dir(QCoreApplication::applicationDirPath()
                   + QStringLiteral("/myVideo"));
    if (!dir.exists())
        return;
    const QStringList files = dir.entryList(
        QStringList() << QStringLiteral("*.mp4") << QStringLiteral("*.mkv")
                      << QStringLiteral("*.wmv") << QStringLiteral("*.avi"),
        QDir::Files, QDir::Name);
    for (const QString &file : files) {
        m_playlist->addMedia(QUrl::fromLocalFile(dir.filePath(file)));
        m_list->addItem(QFileInfo(file).completeBaseName());
    }
}

void VideoPlayer::onPlayClicked()
{
    if (m_playlist->mediaCount() == 0)
        return;
    if (m_player->state() == QMediaPlayer::PlayingState)
        m_player->pause();
    else
        m_player->play();
}

void VideoPlayer::onNextClicked()
{
    if (m_playlist->mediaCount() == 0)
        return;
    m_player->stop();
    m_playlist->next();
    m_player->play();
}

void VideoPlayer::onVolumeUp()
{
    m_volumeSlider->setValue(m_volumeSlider->value() + 5);
    m_player->setVolume(m_volumeSlider->value());
}

void VideoPlayer::onVolumeDown()
{
    m_volumeSlider->setValue(m_volumeSlider->value() - 5);
    m_player->setVolume(m_volumeSlider->value());
}

void VideoPlayer::onFullscreenClicked(bool checked)
{
    /* 全屏模式:隐藏控制条与列表 */
    m_controlBar->setVisible(!checked);
    m_sidePanel->setVisible(!checked);
}

void VideoPlayer::onStateChanged(QMediaPlayer::State state)
{
    const bool playing = (state == QMediaPlayer::PlayingState);
    m_playButton->setChecked(playing);
    m_playButton->setIcon(QIcon(playing
        ? QStringLiteral(":/images/icons/btn_pause.png")
        : QStringLiteral(":/images/icons/btn_play.png")));
}

void VideoPlayer::onPlaylistIndexChanged(int index)
{
    if (index >= 0)
        m_list->setCurrentRow(index);
}

void VideoPlayer::onDurationChanged(qint64 duration)
{
    m_durationSlider->setRange(0, static_cast<int>(duration / 1000));
    m_durationLabel->setText(QStringLiteral("/") + formatTime(duration));
}

void VideoPlayer::onPositionChanged(qint64 position)
{
    if (!m_durationSlider->isSliderDown())
        m_durationSlider->setValue(static_cast<int>(position / 1000));
    m_timeLabel->setText(formatTime(position));
}

void VideoPlayer::onDurationSliderReleased()
{
    m_player->setPosition(m_durationSlider->value() * 1000);
}

void VideoPlayer::onVolumeSliderReleased()
{
    m_player->setVolume(m_volumeSlider->value());
}

void VideoPlayer::onItemClicked(QListWidgetItem *item)
{
    m_player->stop();
    m_playlist->setCurrentIndex(m_list->row(item));
    m_player->play();
}
