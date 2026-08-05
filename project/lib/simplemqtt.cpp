#include "simplemqtt.h"

#include <QDataStream>
#include <QTcpSocket>

namespace {

const quint8 MQTT_CONNECT     = 0x10;
const quint8 MQTT_CONNACK     = 0x20;
const quint8 MQTT_PUBLISH     = 0x30;
const quint8 MQTT_SUBSCRIBE   = 0x82;
const quint8 MQTT_SUBACK      = 0x90;
const quint8 MQTT_PINGREQ     = 0xC0;
const quint8 MQTT_DISCONNECT  = 0xE0;

QByteArray encodeString(const QString &text)
{
    const QByteArray utf8 = text.toUtf8();
    QByteArray out;
    out.append(char((utf8.size() >> 8) & 0xFF));
    out.append(char(utf8.size() & 0xFF));
    out.append(utf8);
    return out;
}

} // namespace

SimpleMqttClient::SimpleMqttClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &SimpleMqttClient::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &SimpleMqttClient::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &SimpleMqttClient::onDisconnected);
}

SimpleMqttClient::~SimpleMqttClient() = default;

void SimpleMqttClient::setHostname(const QString &host)
{
    m_host = host;
}

void SimpleMqttClient::setPort(quint16 port)
{
    m_port = port;
}

SimpleMqttClient::State SimpleMqttClient::state() const
{
    return m_state;
}

void SimpleMqttClient::connectToHost()
{
    if (m_state != Disconnected)
        return;
    m_state = Connecting;
    emit stateChanged();
    m_socket->connectToHost(m_host, m_port);
}

void SimpleMqttClient::disconnectFromHost()
{
    if (m_state == Connected)
        sendPacket(MQTT_DISCONNECT, QByteArray());
    m_socket->disconnectFromHost();
}

void SimpleMqttClient::onConnected()
{
    /* CONNECT 报文:协议名 MQTT + 级别 4 + 干净会话 + keepalive 60 */
    QByteArray payload;
    payload.append(char(0x00));
    payload.append(char(0x04));
    payload.append("MQTT");
    payload.append(char(0x04));
    payload.append(char(0x02));          /* 连接标志:干净会话 */
    payload.append(char(0x00));
    payload.append(char(0x3C));          /* keepalive 60s */
    payload.append(encodeString(QStringLiteral("imx6ull-qt")));
    sendPacket(MQTT_CONNECT, payload);
}

void SimpleMqttClient::sendPacket(quint8 typeAndFlags, const QByteArray &variablePart)
{
    QByteArray packet;
    packet.append(char(typeAndFlags));
    packet.append(encodeRemainingLength(variablePart.size()));
    packet.append(variablePart);
    m_socket->write(packet);
}

QByteArray SimpleMqttClient::encodeRemainingLength(int length)
{
    QByteArray out;
    do {
        quint8 digit = length % 128;
        length /= 128;
        if (length > 0)
            digit |= 0x80;
        out.append(char(digit));
    } while (length > 0);
    return out;
}

void SimpleMqttClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    while (!m_buffer.isEmpty()) {
        const quint8 type = static_cast<quint8>(m_buffer.at(0)) & 0xF0;
        int multiplier = 1;
        int remaining = 0;
        int index = 1;
        bool malformed = false;
        while (index < m_buffer.size() && index < 5) {
            const quint8 digit = static_cast<quint8>(m_buffer.at(index));
            remaining += (digit & 0x7F) * multiplier;
            ++index;
            if ((digit & 0x80) == 0)
                break;
            multiplier *= 128;
            if (index == 5)
                malformed = true;
        }
        if (malformed || index >= m_buffer.size() && remaining > 0)
            return; /* 等待更多数据 */
        if (m_buffer.size() < index + remaining)
            return; /* 包不完整 */

        QByteArray packet = m_buffer.mid(index, remaining);
        m_buffer.remove(0, index + remaining);
        handleIncoming(packet);
        Q_UNUSED(type);
    }
}

void SimpleMqttClient::handleIncoming(const QByteArray &packet)
{
    if (packet.size() < 2)
        return;
    const quint8 type = static_cast<quint8>(packet.at(0)) & 0xF0;

    if (type == 0x20) { /* CONNACK */
        if (packet.size() >= 4 && static_cast<quint8>(packet.at(3)) == 0) {
            m_state = Connected;
            emit stateChanged();
        } else {
            m_state = Disconnected;
            emit stateChanged();
        }
    } else if (type == 0x30) { /* PUBLISH(QoS0) */
        int pos = 2;
        if (packet.size() < pos + 2)
            return;
        const int topicLen = (static_cast<quint8>(packet.at(pos)) << 8)
                           | static_cast<quint8>(packet.at(pos + 1));
        pos += 2;
        if (packet.size() < pos + topicLen)
            return;
        const QString topic = QString::fromUtf8(packet.mid(pos, topicLen));
        pos += topicLen;
        const QByteArray message = packet.mid(pos);
        emit messageReceived(message, topic);
    }
    /* SUBACK / PINGRESP 无需额外处理 */
}

void SimpleMqttClient::onDisconnected()
{
    m_state = Disconnected;
    m_buffer.clear();
    emit stateChanged();
}

bool SimpleMqttClient::subscribe(const QString &topic)
{
    if (m_state != Connected)
        return false;
    QByteArray payload;
    const quint16 id = m_packetId++;
    payload.append(char((id >> 8) & 0xFF));
    payload.append(char(id & 0xFF));
    payload.append(encodeString(topic));
    payload.append(char(0x00)); /* QoS0 */
    sendPacket(MQTT_SUBSCRIBE, payload);
    return true;
}

bool SimpleMqttClient::publish(const QString &topic, const QByteArray &message)
{
    if (m_state != Connected)
        return false;
    QByteArray variable;
    variable.append(encodeString(topic));
    variable.append(message);
    sendPacket(MQTT_PUBLISH, variable);
    return true;
}
