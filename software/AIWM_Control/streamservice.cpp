#include "streamservice.h"
#include <QTimer>


bool StreamService::start(QString cameraIp) {
    qDebug() << "[StreamService] call start()";
    if (m_thread) {
        qDebug() << "[StreamService] Thread already created";
        return false;
    }

    // Reset state
    m_cameraIp = cameraIp;
    m_isStarted = false;
    m_isError = false;

    // Create communication thread
    m_thread = QThread::create([this] { this->threadRun(); });
    if (!m_thread) {
        qDebug() << "[StreamService] Can't create thread object";
        return false;
    }
    m_thread->start();

    // Wait thread
    while (!m_isStarted && !m_isError);
    return m_isStarted;
}
void StreamService::stop() {
    qDebug() << "[StreamService] call stop()";
    if (m_thread) {
        qDebug() << "[StreamService] stop thread...";
        m_eventLoop->exit();
        m_thread->wait(1000);
        m_thread->quit();
        delete m_thread;
        m_thread = nullptr;
    }
}



void StreamService::threadRun() {
    qDebug() << "[StreamService] thread started";
    do {
        // Send GET request
        qDebug() << "[StreamService] camera IP:" << m_cameraIp;
        QNetworkAccessManager accessManager;
        m_requestReply = accessManager.get(QNetworkRequest(QUrl("http://" + m_cameraIp + "/")));
        if (!m_requestReply) {
            qDebug() << "[StreamService] can't create QNetworkReply object";
            m_isError = true;
            break;
        }
        m_requestReply->setReadBufferSize(50 * 1024);
        connect(m_requestReply, &QNetworkReply::readyRead, this, &StreamService::httpDataReceived);

        // Setup timeout timer
        m_timeoutTimer = new (std::nothrow) QTimer;
        connect(m_timeoutTimer, &QTimer::timeout, this, &StreamService::stop);
        m_timeoutTimer->setInterval(2000);
        m_timeoutTimer->setSingleShot(true);
        m_timeoutTimer->start();

        // Start event loop
        m_eventLoop = new QEventLoop;
        if (!m_eventLoop) {
            qDebug() << "[StreamService] can't create QEventLoop object";
            m_isError = true;
            break;
        }
        qDebug() << "[StreamService] start event loop...";
        m_isStarted = true;
        m_eventLoop->exec();
        qDebug() << "[StreamService] event loop stopped";
    } while (0);

    // Free resources
    if (m_eventLoop) {
        delete m_eventLoop;
        m_eventLoop = nullptr;
    }
    /*if (m_requestReply) {
        disconnect(m_requestReply, &QNetworkReply::readyRead, this, &StreamService::httpDataReceived);
        m_requestReply->close();
        m_requestReply->deleteLater();
        m_requestReply = nullptr;
    }*/
    if (m_timeoutTimer) {
        delete m_timeoutTimer;
        m_timeoutTimer = nullptr;
    }
    emit connectionClosed();
}

void StreamService::httpDataReceived() {
    static QString beginSignature("Content-Type: image/jpeg\r\n");
    static QString endSignature("--123456789000000000000987654321\r\n");

    m_timeoutTimer->stop();
    m_timeoutTimer->setInterval(1000);
    m_timeoutTimer->start();

    // Process response data
    if (m_requestReply->error()) {
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
