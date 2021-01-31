#include "swlp.h"
#include "core.h"
#include <QGuiApplication>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QTimer>
#include <QThread>

constexpr const char* SERVER_IP_ADDRESS = "111.111.111.111";
constexpr int SERVER_PORT = 3333;
constexpr int TIMEOUT_VALUE_MS = 2000;



bool Swlp::startThread(void* core) {
    qDebug() << "[Swlp]" << QThread::currentThreadId() << "call start()";

    // Reset state
    m_core = core;
    m_isReady = false;
    m_isError = false;

    // Init thread
    if (QThread::isRunning()) {
        qDebug() << "[Swlp]" << QThread::currentThreadId() << "thread already started";
        stopThread();
        return false;
    }
    QThread::start();

    // Wait thread
    while (!m_isReady && !m_isError);
    return m_isReady;
}
void Swlp::stopThread() {
    qDebug() << "[Swlp]" << QThread::currentThreadId() << "call stop()";
    if (QThread::isRunning()) {
        qDebug() << "[Swlp]" << QThread::currentThreadId() << "stop thread...";
        m_eventLoop->exit();
        QThread::wait(1000);
        QThread::quit();
    }
    qDebug() << "[Swlp]" << QThread::currentThreadId() << "thread stopped";
}



void Swlp::run() {
    qDebug() << "[Swlp]" << QThread::currentThreadId() << "thread started";
    do {
        // Setup UDP socket
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

        // Setup send command payload timer
        QTimer sendTimer;
        connect(&sendTimer, &QTimer::timeout, this, &Swlp::sendCommandPayloadEvent, Qt::ConnectionType::DirectConnection);
        sendTimer.setSingleShot(false);
        sendTimer.setInterval(100);
        sendTimer.start();

        // Setup timeout timer
        m_timeoutTimer = new (std::nothrow) QTimer;
        connect(m_timeoutTimer, &QTimer::timeout, this, &Swlp::stopThread, Qt::ConnectionType::QueuedConnection);
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
    if (m_eventLoop) {
        delete m_eventLoop;
        m_eventLoop = nullptr;
    }
    if (m_socket) {
        delete m_socket;
        m_socket = nullptr;
    }
    emit connectionClosed();
}



void Swlp::datagramReceivedEvent() {
    //qDebug() << "[SWLP]" << QThread::currentThreadId() << "call datagramReceivedEvent()";
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
    reinterpret_cast<Core*>(m_core)->swlpStatusPayloadProcess(&statusPayload);
}
void Swlp::sendCommandPayloadEvent() {
    //qDebug() << "[SWLP]" << QThread::currentThreadId() << "call sendCommandPayloadEvent()";
    swlp_command_payload_t commandPayload;
    reinterpret_cast<Core*>(m_core)->swlpCommandPayloadPrepare(&commandPayload);

    // Make SWLP frame
    swlp_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.start_mark = SWLP_START_MARK_VALUE;
    memcpy(frame.payload, &commandPayload, sizeof(commandPayload));

    // Calculate CRC16
    uint16_t crc = this->calculateCRC16(reinterpret_cast<const uint8_t*>(&frame), sizeof(frame) - 2);
    frame.crc16 = crc;

    // Send SWLP frame
    QNetworkDatagram datagram;
    datagram.setDestination(QHostAddress(SERVER_IP_ADDRESS), SERVER_PORT);
    datagram.setData(QByteArray(reinterpret_cast<const char*>(&frame), sizeof(frame)));
    m_socket->writeDatagram(datagram);
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
