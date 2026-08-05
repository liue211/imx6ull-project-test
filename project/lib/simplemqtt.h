#ifndef SIMPLEMQTT_H
#define SIMPLEMQTT_H

#include <QObject>
#include <QByteArray>
#include <QString>

class QTcpSocket;

/* 轻量 MQTT 3.1.1 客户端(QoS0):
 * 支持 CONNECT / SUBSCRIBE / PUBLISH / DISCONNECT,用于嵌入式演示。 */
class SimpleMqttClient : public QObject
{
    Q_OBJECT

public:
    enum State {
        Disconnected,
        Connecting,
        Connected
    };
    Q_ENUM(State)

    explicit SimpleMqttClient(QObject *parent = nullptr);
    ~SimpleMqttClient() override;

    void setHostname(const QString &host);
    void setPort(quint16 port);
    State state() const;

public slots:
    void connectToHost();
    void disconnectFromHost();
    bool publish(const QString &topic, const QByteArray &payload);
    bool subscribe(const QString &topic);

signals:
    void stateChanged();
    void messageReceived(const QByteArray &message, const QString &topic);

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();

private:
    void sendPacket(quint8 typeAndFlags, const QByteArray &variablePart);
    void handleIncoming(const QByteArray &packet);
    static QByteArray encodeRemainingLength(int length);

    QTcpSocket *m_socket = nullptr;
    QString m_host;
    quint16 m_port = 1883;
    State m_state = Disconnected;
    QByteArray m_buffer;
    quint16 m_packetId = 1;
};

#endif // SIMPLEMQTT_H
