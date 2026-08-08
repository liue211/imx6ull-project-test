#include "musicplay.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
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
    root->setSpacing(12);

    /* 左:标题 + 列表 + 控制按钮 */
    auto *leftCard = new QFrame(this);
    leftCard->setObjectName(QStringLiteral("card"));
    auto *left = new QVBoxLayout(leftCard);
    left->setContentsMargins(12, 12, 12, 12);
    auto *title = new QLabel(QStringLiteral("Q Music,Enjoy it!"), leftCard);
    left->addWidget(title);

    m_list = new QListWidget(leftCard);
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
    m_prevButton->setFixedSize(64, 64);
    m_playButton->setFixedSize(72, 72);
    m_nextButton->setFixedSize(64, 64);
    m_prevButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_prev.png")));
    m_playButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_play.png")));
    m_nextButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_next.png")));
    transport->addStretch();
    transport->addWidget(m_prevButton);
    transport->addWidget(m_playButton);
    transport->addWidget(m_nextButton);
    transport->addStretch();
    left->addLayout(transport);

    root->addWidget(leftCard, 1);

    /* 右:CD + 进度 + 时间 + 小按钮 */
    auto *rightCard = new QFrame(this);
    rightCard->setObjectName(QStringLiteral("card"));
    auto *right = new QVBoxLayout(rightCard);
    right->setContentsMargins(12, 12, 12, 12);
    m_cdLabel = new QLabel(rightCard);
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
    m_timeLabel = new QLabel(QStringLiteral("00:00"), rightCard);
    m_durationLabel = new QLabel(QStringLiteral("00:00"), rightCard);
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
        button->setFixedSize(36, 36);
    m_favoriteButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_favorite.png")));
    m_modeButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_list.png")));
    m_menuButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_menu.png")));
    m_volumeButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_volume.png")));
    smallRow->addStretch();
    smallRow->addWidget(m_favoriteButton);
    smallRow->addWidget(m_modeButton);
    smallRow->addWidget(m_menuButton);
    smallRow->addWidget(m_volumeButton);
    smallRow->addStretch();
    right->addLayout(smallRow);

    root->addWidget(rightCard, 1);
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
    const bool playing = (state == QMediaPlayer::PlayingState);
    m_playButton->setChecked(playing);
    m_playButton->setIcon(QIcon(playing
        ? QStringLiteral(":/images/icons/btn_pause.png")
        : QStringLiteral(":/images/icons/btn_play.png")));
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
