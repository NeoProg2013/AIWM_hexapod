#include "streamservice.h"
#include <QTimer>

StreamService::StreamService(StreamFrameProvider* frameProvider, QObject *parent)
    : QObject(parent), m_frameProvider(frameProvider) {

    file.setFileName("D:/1.txt");
    file.open(QFile::ReadWrite);
}

StreamService::~StreamService() {}

void StreamService::runService(QString cameraIp) {

    if (m_isRunning == true) {
        qDebug() << "m_isRunning == true";
        return;
    }
    qDebug() << "StreamService start. IP:" << cameraIp;

    // Setup timeout timer
    m_timeoutTimer = new QTimer;
    m_timeoutTimer->setInterval(2000);
    m_timeoutTimer->setSingleShot(true);

    // Send GET request
    m_manager = new QNetworkAccessManager();
    m_requestReply = m_manager->get(QNetworkRequest(QUrl("http://" + cameraIp + "/")));
    m_requestReply->setReadBufferSize(50 * 1024);

    // Setup signals
    connect(m_timeoutTimer, &QTimer::timeout, this, &StreamService::httpConnectionClosed);
    connect(m_requestReply, &QNetworkReply::readyRead, this, &StreamService::httpDataReceived);

    // Start timeout timer
    m_timeoutTimer->start();

    // Start event loop
    m_eventLoop = new QEventLoop;
    if (m_eventLoop != nullptr) {
        m_isRunning = true;
        m_eventLoop->exec();
    }
    m_requestReply->close();
    m_requestReply->deleteLater();
    m_timeoutTimer->stop();
    emit connectionClosed();

    // Detach signals
    disconnect(m_timeoutTimer, &QTimer::timeout, this, &StreamService::httpConnectionClosed);
    disconnect(m_requestReply, &QNetworkReply::readyRead, this, &StreamService::httpDataReceived);

    // Free objects 
    delete m_manager;
    m_manager = nullptr;
    delete m_timeoutTimer;
    m_timeoutTimer = nullptr;

    m_isRunning = false;
    qDebug() << "StreamService stop";
}

void StreamService::httpDataReceived() {
    static QString beginSignature("Content-Type: image/jpeg\r\n");
    static QString endSignature("--123456789000000000000987654321\r\n");

    m_timeoutTimer->stop();
    m_timeoutTimer->setInterval(1000);
    m_timeoutTimer->start();

    // Process response data
    if (m_requestReply->error()) {
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
        return;
    }
    chunkData.remove(0, index + signature.size());

    // Check JPEG signature
    if (chunkData.size() >= 2) {
        uint8_t b1 = chunkData.at(0);
        uint8_t b2 = chunkData.at(1);
        if (b1 != 0xFF || b2 != 0xD8) {
            qDebug() << "It is not JPEG";
            return;
        }

        // Update image
        m_frameProvider->setImageRawData(chunkData);
        emit frameReceived();
    }
}

void StreamService::httpConnectionClosed() {
    m_eventLoop->exit();
    emit connectionClosed();
}
