#include "swlp.h"
#include "core.h"
#include <QGuiApplication>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QTimer>
#include <chrono>

constexpr const char* SERVER_IP_ADDRESS = "111.111.111.111";
constexpr int SERVER_PORT = 3333;



bool Swlp::start(void* core) {
    if (m_thread) {
        qDebug() << "[SWLP] Thread already created";
        return false;
    }

    // Reset state
    m_core = core;
    m_isStarted = false;
    m_isError = false;

    // Create communication thread
    m_thread = QThread::create([this] { this->threadRun(); });
    if (!m_thread) {
        qDebug() << "[SWLP] Can't create thread object";
        return false;
    }
    m_thread->start();

    // Wait thread
    while (!m_isStarted && !m_isError);
    return m_isStarted;
}
void Swlp::stop() {
    if (m_thread) {
        m_eventLoop->exit();
        m_thread->wait(1000);
        m_thread->quit();
        delete m_thread;
        m_thread = nullptr;
    }
}




void Swlp::threadRun() {
    do {
        // Setup UDP socket
        m_socket = new (std::nothrow) QUdpSocket();
        if (!m_socket) {
            qDebug() << "[SWLP] can't create QUdpSocket";
            m_isError = true;
            break;
        }
        connect(m_socket, &QUdpSocket::readyRead, this, &Swlp::datagramReceivedEvent, Qt::ConnectionType::DirectConnection);
        if (m_socket->bind(SERVER_PORT) == false) {
            qDebug() << "[SWLP] can't bind socket to port";
            m_isError = true;
            break;
        }

        // Setup send command payload timer
        QTimer sendTimer;
        connect(&sendTimer, &QTimer::timeout, this, &Swlp::sendCommandPayloadEvent, Qt::ConnectionType::DirectConnection);
        sendTimer.setSingleShot(false);
        sendTimer.setInterval(100);
        sendTimer.start();

        // Start event loop
        m_eventLoop = new (std::nothrow) QEventLoop;
        if (!m_eventLoop) {
            qDebug() << "[SWLP] can't create QEventLoop";
            m_isError = true;
            break;
        }
        qDebug() << "[SWLP] start event loop...";
        if (m_eventLoop != nullptr) {
            m_isStarted = true;
            m_eventLoop->exec();
        }
        qDebug() << "[SWLP] event loop stopped";

        // Stop send command payload timer
        sendTimer.stop();
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
}



void Swlp::datagramReceivedEvent() {
    //qDebug() << "[SWLP] call datagramReceivedEvent()";
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
    //qDebug() << "[SWLP] call sendCommandPayloadEvent()";
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
