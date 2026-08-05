#include "musicplay.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMediaPlaylist>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

MusicPlay::MusicPlay(QWidget *parent)
    : QWidget(parent)
{
    QFile qss(QStringLiteral(":/style/music_style.qss"));
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

    connect(m_playButton, &QPushButton::clicked, this, &MusicPlay::onPlayClicked);
    connect(m_nextButton, &QPushButton::clicked, this, &MusicPlay::onNextClicked);
    connect(m_prevButton, &QPushButton::clicked, this, &MusicPlay::onPrevClicked);
    connect(m_player, &QMediaPlayer::stateChanged, this, &MusicPlay::onStateChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MusicPlay::onDurationChanged);
    connect(m_player, &QMediaPlayer::positionChanged, this, &MusicPlay::onPositionChanged);
    connect(m_playlist, &QMediaPlaylist::currentIndexChanged,
            this, &MusicPlay::onPlaylistIndexChanged);
    connect(m_list, &QListWidget::itemClicked, this, &MusicPlay::onItemClicked);
    connect(m_durationSlider, &QSlider::sliderReleased, this, &MusicPlay::onSliderReleased);

    scanSongs();
}

void MusicPlay::setupUi()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);

    /* 左:标题 + 列表 + 控制按钮 */
    auto *left = new QVBoxLayout();
    auto *title = new QLabel(QStringLiteral("Q Music,Enjoy it!"), this);
    title->setStyleSheet(QStringLiteral("color:white; font-size:16px;"));
    left->addWidget(title);

    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("listWidget"));
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    left->addWidget(m_list, 1);

    auto *transport = new QHBoxLayout();
    m_prevButton = new QPushButton(this);
    m_playButton = new QPushButton(this);
    m_nextButton = new QPushButton(this);
    m_prevButton->setObjectName(QStringLiteral("btn_previous"));
    m_playButton->setObjectName(QStringLiteral("btn_play"));
    m_nextButton->setObjectName(QStringLiteral("btn_next"));
    m_playButton->setCheckable(true);
    for (QPushButton *button : {m_prevButton, m_playButton, m_nextButton})
        button->setFixedSize(64, 64);
    transport->addStretch();
    transport->addWidget(m_prevButton);
    transport->addWidget(m_playButton);
    transport->addWidget(m_nextButton);
    transport->addStretch();
    left->addLayout(transport);

    root->addLayout(left, 1);

    /* 右:CD + 进度 + 时间 + 小按钮 */
    auto *right = new QVBoxLayout();
    m_cdLabel = new QLabel(this);
    m_cdLabel->setAlignment(Qt::AlignCenter);
    m_cdLabel->setFixedSize(260, 260);
    m_cdLabel->setPixmap(QPixmap(QStringLiteral(":/images/music_pic/cd.png"))
                             .scaled(m_cdLabel->size(), Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation));
    right->addWidget(m_cdLabel, 1);

    m_durationSlider = new QSlider(Qt::Horizontal, this);
    m_durationSlider->setObjectName(QStringLiteral("durationSlider"));
    right->addWidget(m_durationSlider);

    auto *timeRow = new QHBoxLayout();
    m_timeLabel = new QLabel(QStringLiteral("00:00"), this);
    m_durationLabel = new QLabel(QStringLiteral("00:00"), this);
    m_timeLabel->setStyleSheet(QStringLiteral("color:white;"));
    m_durationLabel->setStyleSheet(QStringLiteral("color:white;"));
    timeRow->addWidget(m_timeLabel);
    timeRow->addStretch();
    timeRow->addWidget(m_durationLabel);
    right->addLayout(timeRow);

    auto *smallRow = new QHBoxLayout();
    m_favoriteButton = new QPushButton(this);
    m_modeButton = new QPushButton(this);
    m_menuButton = new QPushButton(this);
    m_volumeButton = new QPushButton(this);
    m_favoriteButton->setObjectName(QStringLiteral("btn_favorite"));
    m_modeButton->setObjectName(QStringLiteral("btn_mode"));
    m_menuButton->setObjectName(QStringLiteral("btn_menu"));
    m_volumeButton->setObjectName(QStringLiteral("btn_volume"));
    m_favoriteButton->setCheckable(true);
    for (QPushButton *button : {m_favoriteButton, m_modeButton, m_menuButton, m_volumeButton})
        button->setFixedSize(32, 32);
    smallRow->addStretch();
    smallRow->addWidget(m_favoriteButton);
    smallRow->addWidget(m_modeButton);
    smallRow->addWidget(m_menuButton);
    smallRow->addWidget(m_volumeButton);
    smallRow->addStretch();
    right->addLayout(smallRow);

    root->addLayout(right, 1);
}

QString MusicPlay::formatTime(qint64 ms)
{
    const int totalSeconds = static_cast<int>(ms / 1000);
    return QStringLiteral("%1:%2")
        .arg(totalSeconds / 60, 2, 10, QLatin1Char('0'))
        .arg(totalSeconds % 60, 2, 10, QLatin1Char('0'));
}

void MusicPlay::scanSongs()
{
    const QDir dir(QCoreApplication::applicationDirPath()
                   + QStringLiteral("/myMusic"));
    if (!dir.exists())
        return;
    const QStringList files = dir.entryList(
        QStringList() << QStringLiteral("*.mp3") << QStringLiteral("*.flac")
                      << QStringLiteral("*.wav"),
        QDir::Files, QDir::Name);
    for (const QString &file : files) {
        m_playlist->addMedia(QUrl::fromLocalFile(dir.filePath(file)));
        QFileInfo info(file);
        m_list->addItem(info.completeBaseName());
    }
}

void MusicPlay::onPlayClicked()
{
    if (m_playlist->mediaCount() == 0)
        return;
    if (m_player->state() == QMediaPlayer::PlayingState)
        m_player->pause();
    else
        m_player->play();
}

void MusicPlay::onNextClicked()
{
    if (m_playlist->mediaCount() == 0)
        return;
    m_player->stop();
    m_playlist->next();
    m_player->play();
}

void MusicPlay::onPrevClicked()
{
    if (m_playlist->mediaCount() == 0)
        return;
    m_player->stop();
    m_playlist->previous();
    m_player->play();
}

void MusicPlay::onStateChanged(QMediaPlayer::State state)
{
    m_playButton->setChecked(state == QMediaPlayer::PlayingState);
}

void MusicPlay::onPlaylistIndexChanged(int index)
{
    if (index >= 0)
        m_list->setCurrentRow(index);
}

void MusicPlay::onDurationChanged(qint64 duration)
{
    m_durationSlider->setRange(0, static_cast<int>(duration / 1000));
    m_durationLabel->setText(formatTime(duration));
}

void MusicPlay::onPositionChanged(qint64 position)
{
    if (!m_durationSlider->isSliderDown())
        m_durationSlider->setValue(static_cast<int>(position / 1000));
    m_timeLabel->setText(formatTime(position));
}

void MusicPlay::onSliderReleased()
{
    m_player->setPosition(m_durationSlider->value() * 1000);
}

void MusicPlay::onItemClicked(QListWidgetItem *item)
{
    m_player->stop();
    m_playlist->setCurrentIndex(m_list->row(item));
    m_player->play();
}
