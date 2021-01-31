#include "streamservice.h"
#include <QTimer>

constexpr int TIMEOUT_VALUE_MS = 2000;


bool StreamService::startThread(QString cameraIp) {
    qDebug() << "[StreamService]" << QThread::currentThreadId() << "call start()";

    // Reset state
    m_cameraIp = cameraIp;
    m_isReady = false;
    m_isError = false;
    m_buffer.clear();

    // Init thread
    if (QThread::isRunning()) {
        qDebug() << "[StreamService]" << QThread::currentThreadId() << "thread already started";
        stopThread();
        return false;
    }
    QThread::start();

    // Wait thread
    while (!m_isReady && !m_isError);
    return m_isReady;
}
void StreamService::stopThread() {
    qDebug() << "[StreamService]" << QThread::currentThreadId() << "call stop()";
    if (QThread::isRunning()) {
        qDebug() << "[StreamService]" << QThread::currentThreadId() << "stop thread...";
        m_eventLoop->exit();
        QThread::wait(1000);
        QThread::quit();
    }
    qDebug() << "[StreamService]" << QThread::currentThreadId() << "thread stopped";
}



void StreamService::run() {
    qDebug() << "[StreamService]" << QThread::currentThreadId() << "thread started";
    do {
        // Send GET request
        qDebug() << "[StreamService]" << QThread::currentThreadId() << "camera IP:" << m_cameraIp;
        QNetworkAccessManager accessManager;
        m_requestReply = accessManager.get(QNetworkRequest(QUrl("http://" + m_cameraIp + "/")));
        if (!m_requestReply) {
            qDebug() << "[StreamService]" << QThread::currentThreadId() << "can't create QNetworkReply object";
            m_isError = true;
            break;
        }
        m_requestReply->setReadBufferSize(60 * 1024);
        connect(m_requestReply, &QNetworkReply::readyRead, this, &StreamService::httpDataReceived, Qt::ConnectionType::DirectConnection);

        // Setup timeout timer
        m_timeoutTimer = new (std::nothrow) QTimer;
        connect(m_timeoutTimer, &QTimer::timeout, this, &StreamService::stopThread, Qt::ConnectionType::QueuedConnection);
        m_timeoutTimer->setInterval(TIMEOUT_VALUE_MS);
        m_timeoutTimer->setSingleShot(true);
        m_timeoutTimer->start();

        // Start event loop
        m_eventLoop = new QEventLoop;
        if (!m_eventLoop) {
            qDebug() << "[StreamService]" << QThread::currentThreadId() << "can't create QEventLoop object";
            m_isError = true;
            break;
        }
        qDebug() << "[StreamService]" << QThread::currentThreadId() << "start event loop...";
        m_isReady = true;
        m_eventLoop->exec();
        qDebug() << "[StreamService]" << QThread::currentThreadId() << "event loop stopped";
    } while (0);

    // Free resources
    if (m_eventLoop) {
        delete m_eventLoop;
        m_eventLoop = nullptr;
    }
    if (m_timeoutTimer) {
        delete m_timeoutTimer;
        m_timeoutTimer = nullptr;
    }
    emit connectionClosed();
}

void StreamService::httpDataReceived() {
    //qDebug() << "[StreamService]" << QThread::currentThreadId() << "call httpDataReceived()";
    static QString beginSignature("Content-Type: image/jpeg\r\n");
    static QString endSignature("--123456789000000000000987654321\r\n");

    m_timeoutTimer->stop();
    m_timeoutTimer->setInterval(TIMEOUT_VALUE_MS);
    m_timeoutTimer->start();

    // Process response data
    if (m_requestReply->error()) {
        qDebug() << "m_requestReply->error() == true";
        emit badFrameReceived();
        return;
    }

    // Read response and check end of chunk signature
    m_buffer.append(m_requestReply->readAll());
    int endIndex = m_buffer.indexOf("--123456789000000000000987654321\r\n");
    if (endIndex == -1) {
        return;
    }

    // Search begin of chunk
    int beginIndex = m_buffer.indexOf(beginSignature);
    if (beginIndex > endIndex) {
        qDebug() << "Something wrong";
        m_buffer.clear();
        emit badFrameReceived();
        return;
    }

    // Erase chunk
    QByteArray chunkData = m_buffer.mid(beginIndex, endIndex - beginIndex);
    m_buffer.remove(beginIndex, endIndex - beginIndex + endSignature.size());

    // Parse chunk data
    QString signature("\r\n\r\n");
    int index = chunkData.indexOf(signature, beginIndex);
    if (index == -1) {
        qDebug() << "Something wrong";
        m_buffer.clear();
        emit badFrameReceived();
        return;
    }
    chunkData.remove(0, index + signature.size());

    // Check JPEG signature
    if (chunkData.size() >= 2) {
        uint8_t b1 = chunkData.at(0);
        uint8_t b2 = chunkData.at(1);
        if (b1 != 0xFF || b2 != 0xD8) {
            qDebug() << "It is not JPEG";
            emit badFrameReceived();
            return;
        }

        // Update image
        m_frameProvider->setImageRawData(chunkData);
        emit frameReceived();
    }
}
