#include "swlp.h"
#include <QHostAddress>
#include <QNetworkDatagram>

constexpr const char* SERVER_IP_ADDRESS = "111.111.111.111";
constexpr int SERVER_PORT = 3333;
constexpr int TIMEOUT_VALUE_MS = 2000;


bool Swlp::startService() {
    qDebug() << "[Swlp]" << QThread::currentThreadId() << "call start()";

    // Reset state
    m_isReady = false;
    m_isError = false;

    // Clear payload
    m_payloadMutex.lock();
    memset(&m_commandPayload, 0, sizeof(m_commandPayload));
    m_commandPayload.command = SWLP_CMD_NONE;
    m_payloadMutex.unlock();

    // Init thread
    stopService();
    QThread::start();

    // Wait thread
    while (!m_isReady && !m_isError);
    return m_isReady;
}
void Swlp::stopService() {
    qDebug() << "[Swlp]" << QThread::currentThreadId() << "call stop()";
    if (QThread::isRunning()) {
        qDebug() << "[Swlp]" << QThread::currentThreadId() << "stop thread...";
        m_eventLoopMutex.lock();
        if (m_eventLoop) {
            m_eventLoop->exit();
        }
        m_eventLoopMutex.unlock();
        QThread::wait(1000);
        QThread::quit();
    }
    qDebug() << "[Swlp]" << QThread::currentThreadId() << "thread stopped";
}
void Swlp::sendGetUpCommand()       { setCommand(SWLP_CMD_SELECT_SEQUENCE_UP);           }
void Swlp::sendGetDownCommand()     { setCommand(SWLP_CMD_SELECT_SEQUENCE_DOWN);         }
void Swlp::sendUpDownCommand()      { setCommand(SWLP_CMD_SELECT_SEQUENCE_UP_DOWN);      }
void Swlp::sendPushPullCommand()    { setCommand(SWLP_CMD_SELECT_SEQUENCE_PUSH_PULL);    }
void Swlp::sendAttackLeftCommand()  { setCommand(SWLP_CMD_SELECT_SEQUENCE_ATTACK_LEFT);  }
void Swlp::sendAttackRightCommand() { setCommand(SWLP_CMD_SELECT_SEQUENCE_ATTACK_RIGHT); }
void Swlp::sendDanceCommand()       { setCommand(SWLP_CMD_SELECT_SEQUENCE_DANCE);        }
void Swlp::sendRotateXCommand()     { setCommand(SWLP_CMD_SELECT_SEQUENCE_ROTATE_X);     }
void Swlp::sendRotateZCommand()     { setCommand(SWLP_CMD_SELECT_SEQUENCE_ROTATE_Z);     }
void Swlp::sendStopMoveCommand()    { setCommand(SWLP_CMD_SELECT_SEQUENCE_NONE);         }
void Swlp::sendStartMotionCommand(QVariant speed, QVariant stepLength, QVariant curvature) {
    int16_t stepLengthInt16 = stepLength.toInt();
    m_payloadMutex.lock();
    m_commandPayload.command = (stepLengthInt16 < 0) ? SWLP_CMD_SELECT_SEQUENCE_REVERSE : SWLP_CMD_SELECT_SEQUENCE_DIRECT;
    m_commandPayload.step_length = abs(stepLengthInt16);
    m_commandPayload.curvature = curvature.toInt();
    m_commandPayload.motion_speed = speed.toInt();
    m_payloadMutex.unlock();
}



void Swlp::run() {
    qDebug() << "[Swlp]" << QThread::currentThreadId() << "thread started";
    do {
        // Setup UDP socket (callbacks call from this thread)
        m_socket = new (std::nothrow) QUdpSocket();
        if (!m_socket) {
            qDebug() << "[Swlp]" << QThread::currentThreadId() << "can't create QUdpSocket object";
            m_isError = true;
            break;
        }
        connect(m_socket, &QUdpSocket::readyRead, this, &Swlp::datagramReceivedEvent, Qt::ConnectionType::DirectConnection);
        if (m_socket->bind(SERVER_PORT) == false) {
            qDebug() << "[Swlp]" << QThread::currentThreadId() << "can't bind socket to port";
            m_isError = true;
            break;
        }

        // Setup send command payload timer (callbacks call from this thread)
        QTimer sendTimer;
        connect(&sendTimer, &QTimer::timeout, this, &Swlp::sendCommandPayloadEvent, Qt::ConnectionType::DirectConnection);
        sendTimer.setSingleShot(false);
        sendTimer.setInterval(100);
        sendTimer.start();

        // Setup timeout timer (callback call from GUI thread)
        m_timeoutTimer = new (std::nothrow) QTimer;
        connect(m_timeoutTimer, &QTimer::timeout, this, &Swlp::stopService, Qt::ConnectionType::QueuedConnection);
        m_timeoutTimer->setInterval(TIMEOUT_VALUE_MS);
        m_timeoutTimer->setSingleShot(true);
        m_timeoutTimer->start();

        // Start event loop
        m_eventLoop = new (std::nothrow) QEventLoop;
        if (!m_eventLoop) {
            qDebug() << "[Swlp]" << QThread::currentThreadId() << "can't create QEventLoop object";
            m_isError = true;
            break;
        }
        qDebug() << "[Swlp]" << QThread::currentThreadId() << "start event loop...";
        m_isReady = true;
        m_eventLoop->exec();
        qDebug() << "[Swlp]" << QThread::currentThreadId() << "event loop stopped";
    } while (0);

    // Free resources
    m_eventLoopMutex.lock();
    if (m_eventLoop) {
        delete m_eventLoop;
        m_eventLoop = nullptr;
    }
    m_eventLoopMutex.unlock();

    if (m_socket) {
        delete m_socket;
        m_socket = nullptr;
    }
    emit connectionClosed();
}



void Swlp::datagramReceivedEvent() {
    //qDebug() << "[Swlp]" << QThread::currentThreadId() << "call datagramReceivedEvent()";
    // Reset timeout timer
    m_timeoutTimer->stop();
    m_timeoutTimer->setInterval(TIMEOUT_VALUE_MS);
    m_timeoutTimer->start();

    // Check datagram size
    qint64 datagram_size = m_socket->pendingDatagramSize();
    if (datagram_size != sizeof(swlp_frame_t)) {
        swlp_frame_t swlp_frame;
        m_socket->readDatagram(reinterpret_cast<char*>(&swlp_frame), 0);
        return;
    }

    // Read frame
    swlp_frame_t swlp_frame;
    m_socket->readDatagram(reinterpret_cast<char*>(&swlp_frame), sizeof(swlp_frame));

    // Verify SWLP frame
    if (swlp_frame.start_mark != SWLP_START_MARK_VALUE) {
        return;
    }
    if (this->calculateCRC16(reinterpret_cast<const uint8_t*>(&swlp_frame), sizeof(swlp_frame)) != 0) {
        return;
    }

    // Process status payload
    swlp_status_payload_t statusPayload;
    memcpy(&statusPayload, swlp_frame.payload, sizeof(statusPayload));
    emit frameReceived();
    emit systemStatusUpdated(statusPayload.system_status, statusPayload.module_status);
    emit batteryStatusUpdated(statusPayload.battery_charge, statusPayload.battery_voltage);
}
void Swlp::sendCommandPayloadEvent() {
    //qDebug() << "[Swlp]" << QThread::currentThreadId() << "call sendCommandPayloadEvent()";
    // Prepare SWLP frame
    swlp_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.start_mark = SWLP_START_MARK_VALUE;

    // Copy payload to frame
    m_payloadMutex.lock();
    memcpy(frame.payload, &m_commandPayload, sizeof(m_commandPayload));
    m_payloadMutex.unlock();

    // Calculate CRC16
    uint16_t crc = this->calculateCRC16(reinterpret_cast<const uint8_t*>(&frame), sizeof(frame) - 2);
    frame.crc16 = crc;

    // Send SWLP frame
    QNetworkDatagram datagram;
    datagram.setDestination(QHostAddress(SERVER_IP_ADDRESS), SERVER_PORT);
    datagram.setData(QByteArray(reinterpret_cast<const char*>(&frame), sizeof(frame)));
    m_socket->writeDatagram(datagram);
}
void Swlp::setCommand(uint8_t command) {
    m_payloadMutex.lock();
    m_commandPayload.command = command;
    m_payloadMutex.unlock();
}
uint16_t Swlp::calculateCRC16(const uint8_t* frame, int size) {
    uint16_t crc16 = 0xFFFF;
    uint16_t data = 0;
    uint16_t k = 0;

    while (size--) {
        crc16 ^= *frame++;
        k = 8;
        while (k--) {
            data = crc16;
            crc16 >>= 1;
            if (data & 0x0001) {
                crc16 ^= SWLP_CRC16_POLYNOM;
            }
        }
    }
    return crc16;
}
