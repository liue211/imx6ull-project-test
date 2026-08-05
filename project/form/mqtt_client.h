#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <QWidget>

class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class SimpleMqttClient;

/* MQTT 发布/订阅客户端(内置轻量 MQTT 3.1.1 实现) */
class mqtt_client : public QWidget
{
    Q_OBJECT

public:
    explicit mqtt_client(QWidget *parent = nullptr);

private slots:
    void onConnectClicked();
    void onSubscribeClicked();
    void onPublishClicked();
    void onStateChanged();

private:
    void setupUi();
    void appendLog(const QString &text);

    SimpleMqttClient *m_client = nullptr;
    QLineEdit *m_hostEdit = nullptr;
    QSpinBox *m_portSpin = nullptr;
    QPushButton *m_connectButton = nullptr;
    QLineEdit *m_subscribeTopicEdit = nullptr;
    QPushButton *m_subscribeButton = nullptr;
    QLineEdit *m_publishTopicEdit = nullptr;
    QLineEdit *m_messageEdit = nullptr;
    QPushButton *m_publishButton = nullptr;
    QPlainTextEdit *m_log = nullptr;
};

#endif // MQTT_CLIENT_H
