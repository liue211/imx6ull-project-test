#include "mqtt_client.h"

#include "simplemqtt.h"

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

mqtt_client::mqtt_client(QWidget *parent)
    : QWidget(parent)
{
    setupUi();

    m_client = new SimpleMqttClient(this);
    m_client->setHostname(m_hostEdit->text());
    m_client->setPort(static_cast<quint16>(m_portSpin->value()));

    connect(m_client, &SimpleMqttClient::stateChanged,
            this, &mqtt_client::onStateChanged);
    connect(m_client, &SimpleMqttClient::messageReceived, this,
            [this](const QByteArray &message, const QString &topic) {
        appendLog(QStringLiteral("%1 --- 收到 Topic: %2 Message: %3")
                      .arg(QDateTime::currentDateTime().toString(Qt::ISODate),
                           topic, QString::fromUtf8(message)));
    });
    connect(m_hostEdit, &QLineEdit::textChanged, m_client,
            &SimpleMqttClient::setHostname);
    connect(m_portSpin, qOverload<int>(&QSpinBox::valueChanged), this,
            [this](int port) { m_client->setPort(static_cast<quint16>(port)); });

    connect(m_connectButton, &QPushButton::clicked, this, &mqtt_client::onConnectClicked);
    connect(m_subscribeButton, &QPushButton::clicked, this, &mqtt_client::onSubscribeClicked);
    connect(m_publishButton, &QPushButton::clicked, this, &mqtt_client::onPublishClicked);
}

void mqtt_client::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);

    auto *connBox = new QGroupBox(QStringLiteral("连接"), this);
    auto *connLayout = new QHBoxLayout(connBox);
    m_hostEdit = new QLineEdit(QStringLiteral("127.0.0.1"), connBox);
    m_portSpin = new QSpinBox(connBox);
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(1883);
    m_connectButton = new QPushButton(QStringLiteral("Connect"), connBox);
    m_statusLabel = new QLabel(QStringLiteral("未连接"), connBox);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    m_connectButton->setObjectName(QStringLiteral("btn_connect"));
    connLayout->addWidget(new QLabel(QStringLiteral("Host:"), connBox));
    connLayout->addWidget(m_hostEdit, 1);
    connLayout->addWidget(new QLabel(QStringLiteral("Port:"), connBox));
    connLayout->addWidget(m_portSpin);
    connLayout->addWidget(m_statusLabel);
    connLayout->addWidget(m_connectButton);
    root->addWidget(connBox);

    auto *subBox = new QGroupBox(QStringLiteral("订阅"), this);
    auto *subLayout = new QHBoxLayout(subBox);
    m_subscribeTopicEdit = new QLineEdit(QStringLiteral("test/topic"), subBox);
    m_subscribeButton = new QPushButton(QStringLiteral("Subscribe"), subBox);
    m_subscribeButton->setObjectName(QStringLiteral("btn_subscribe"));
    subLayout->addWidget(new QLabel(QStringLiteral("Topic:"), subBox));
    subLayout->addWidget(m_subscribeTopicEdit, 1);
    subLayout->addWidget(m_subscribeButton);
    root->addWidget(subBox);

    auto *pubBox = new QGroupBox(QStringLiteral("发布"), this);
    auto *pubLayout = new QGridLayout(pubBox);
    m_publishTopicEdit = new QLineEdit(QStringLiteral("test/topic"), pubBox);
    m_messageEdit = new QLineEdit(QStringLiteral("hello"), pubBox);
    m_publishButton = new QPushButton(QStringLiteral("Publish"), pubBox);
    m_publishButton->setObjectName(QStringLiteral("btn_publish"));
    pubLayout->addWidget(new QLabel(QStringLiteral("Topic:"), pubBox), 0, 0);
    pubLayout->addWidget(m_publishTopicEdit, 0, 1);
    pubLayout->addWidget(new QLabel(QStringLiteral("Message:"), pubBox), 1, 0);
    pubLayout->addWidget(m_messageEdit, 1, 1);
    pubLayout->addWidget(m_publishButton, 1, 2);
    root->addWidget(pubBox);

    m_log = new QPlainTextEdit(this);
    m_log->setObjectName(QStringLiteral("logEdit"));
    m_log->setReadOnly(true);
    m_log->setPlaceholderText(QStringLiteral("连接/订阅/发布日志"));
    root->addWidget(m_log, 1);
}

void mqtt_client::appendLog(const QString &text)
{
    m_log->appendPlainText(text);
}

void mqtt_client::onStateChanged()
{
    const SimpleMqttClient::State state = m_client->state();
    const QString text = [state]() {
        switch (state) {
        case SimpleMqttClient::Disconnected: return QStringLiteral("未连接");
        case SimpleMqttClient::Connecting:   return QStringLiteral("连接中");
        case SimpleMqttClient::Connected:    return QStringLiteral("已连接");
        }
        return QStringLiteral("未知");
    }();
    appendLog(QStringLiteral("%1 --- 状态变化: %2")
                  .arg(QDateTime::currentDateTime().toString(Qt::ISODate), text));

    m_connectButton->setText(state == SimpleMqttClient::Connected
                                 ? QStringLiteral("Disconnect")
                                 : QStringLiteral("Connect"));
    m_statusLabel->setText(text);
    if (state == SimpleMqttClient::Connected)
        m_statusLabel->setStyleSheet(QStringLiteral("color:#22C55E;"));
    else if (state == SimpleMqttClient::Connecting)
        m_statusLabel->setStyleSheet(QStringLiteral("color:#F59E0B;"));
    else
        m_statusLabel->setStyleSheet(QStringLiteral("color:#6B7280;"));
}

void mqtt_client::onConnectClicked()
{
    if (m_client->state() == SimpleMqttClient::Disconnected)
        m_client->connectToHost();
    else
        m_client->disconnectFromHost();
}

void mqtt_client::onSubscribeClicked()
{
    if (!m_client->subscribe(m_subscribeTopicEdit->text())) {
        QMessageBox::critical(this, QStringLiteral("错误"),
                              QStringLiteral("订阅失败:请确认已连接。"));
        return;
    }
    appendLog(QStringLiteral("%1 --- 订阅 Topic: %2")
                  .arg(QDateTime::currentDateTime().toString(Qt::ISODate),
                       m_subscribeTopicEdit->text()));
}

void mqtt_client::onPublishClicked()
{
    if (!m_client->publish(m_publishTopicEdit->text(),
                           m_messageEdit->text().toUtf8())) {
        QMessageBox::critical(this, QStringLiteral("错误"),
                              QStringLiteral("发布失败:请确认已连接。"));
        return;
    }
    appendLog(QStringLiteral("%1 --- 发布 Topic: %2 Message: %3")
                  .arg(QDateTime::currentDateTime().toString(Qt::ISODate),
                       m_publishTopicEdit->text(), m_messageEdit->text()));
}
