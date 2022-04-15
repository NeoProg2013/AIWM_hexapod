#include "swlp.h"
#include <cstring>
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
    std::memset(&m_swlpRequst, 0, sizeof(m_swlpRequst));

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

void Swlp::sendMotionCommand(QVariant speed, QVariant distance, QVariant curvature, QVariant stepHeight,
                             QVariant surfacePointX, QVariant surfacePointY, QVariant surfacePointZ,
                             QVariant surfaceRotateX, QVariant surfaceRotateY, QVariant surfaceRotateZ,
                             QVariant isStabEnabled) {
    m_requestMutex.lock();
    m_swlpRequst.speed = speed.toInt();
    m_swlpRequst.curvature = curvature.toInt();
    m_swlpRequst.distance = distance.toInt();
    m_swlpRequst.step_height = stepHeight.toInt();

    if (isStabEnabled.toBool()) {
        m_swlpRequst.motion_ctrl |= SWLP_MOTION_CTRL_EN_STAB;
    } else {
        m_swlpRequst.motion_ctrl &= ~SWLP_MOTION_CTRL_EN_STAB;
    }

    m_swlpRequst.surface_point_x = surfacePointX.toInt();
    m_swlpRequst.surface_point_y = surfacePointY.toInt();
    m_swlpRequst.surface_point_z = surfacePointZ.toInt();

    m_swlpRequst.surface_rotate_x = surfaceRotateX.toInt();
    m_swlpRequst.surface_rotate_y = surfaceRotateY.toInt();
    m_swlpRequst.surface_rotate_z = surfaceRotateZ.toInt();

    qDebug() << "[sendMotionCommand] speed" << m_swlpRequst.speed;
    qDebug() << "[sendMotionCommand] curvature" << m_swlpRequst.curvature;
    qDebug() << "[sendMotionCommand] distance" << m_swlpRequst.distance;
    qDebug() << "[sendMotionCommand] step_height" << m_swlpRequst.step_height;
    qDebug() << "[sendMotionCommand] motion_ctrl" << m_swlpRequst.motion_ctrl;
    qDebug() << "[sendMotionCommand] surface_point_x" << m_swlpRequst.surface_point_x;
    qDebug() << "[sendMotionCommand] surface_point_y" << m_swlpRequst.surface_point_y;
    qDebug() << "[sendMotionCommand] surface_point_z" << m_swlpRequst.surface_point_z;
    qDebug() << "[sendMotionCommand] surface_rotate_x" << m_swlpRequst.surface_rotate_x;
    qDebug() << "[sendMotionCommand] surface_rotate_y" << m_swlpRequst.surface_rotate_y;
    qDebug() << "[sendMotionCommand] surface_rotate_z" << m_swlpRequst.surface_rotate_z;

    m_requestMutex.unlock();
}


void Swlp::run() {
    qDebug() << "[Swlp]" << QThread::currentThreadId() << "thread started";

    // Clear payload
    m_requestMutex.lock();
    memset(&m_swlpRequst, 0, sizeof(m_swlpRequst));
    m_requestMutex.unlock();

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
        sendTimer.setInterval(30);
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
    //m_timeoutTimer->start();

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
    if (swlp_frame.start_mark != SWLP_START_MARK_VALUE || swlp_frame.version != SWLP_CURRENT_VERSION) {
        qDebug() << "[Swlp]" << QThread::currentThreadId() << "wrong start mark or version";
        return;
    }
    if (this->calculateCRC16(reinterpret_cast<const uint8_t*>(&swlp_frame), sizeof(swlp_frame)) != 0) {
        qDebug() << "[Swlp]" << QThread::currentThreadId() << "wrong CRC value";
        return;
    }

    // Process status payload
    swlp_response_t response;
    memcpy(&response, swlp_frame.payload, sizeof(response));
    emit frameReceived();
    emit systemStatusUpdated(response.system_status, response.module_status);
    emit batteryStatusUpdated(response.battery_charge, response.battery_voltage);
}
void Swlp::sendCommandPayloadEvent() {
    //qDebug() << "[Swlp]" << QThread::currentThreadId() << "call sendCommandPayloadEvent() ";
    // Prepare SWLP frame
    swlp_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.start_mark = SWLP_START_MARK_VALUE;
    frame.version = SWLP_CURRENT_VERSION;

    // Copy payload to frame
    m_requestMutex.lock();
    memcpy(frame.payload, &m_swlpRequst, sizeof(m_swlpRequst));
    m_requestMutex.unlock();

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
