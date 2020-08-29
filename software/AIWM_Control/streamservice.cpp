#include "streamservice.h"
#include <QTimer>

StreamService::StreamService(StreamFrameProvider* frameProvider, QObject *parent)
    : QObject(parent), m_frameProvider(frameProvider) {}

StreamService::~StreamService() {}

void StreamService::runService(QString cameraIp) {

    if (m_isRunning == true) {
        qDebug() << "m_isRunning == true";
        return;
    }
    qDebug() << "StreamService start. IP: " << cameraIp;

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

    // Refresh timeout timer
    m_timeoutTimer->stop();
    m_timeoutTimer->setInterval(1000);
    m_timeoutTimer->start();


    //
    // Process response data
    //

    if (m_requestReply->error()) {
        return;
    }

    static bool isHeaderReceived = false;
    static int processedBytesCount = 0;
    static int jpegSize = 0;
    static QByteArray m_imageBuffer;

    // Read response
    QByteArray response = m_requestReply->readAll();
    QString responseStr(response);

    // Get JPEG size
    int index = responseStr.indexOf("Content-Length");
    if (index != -1) {

        // Reset state
        isHeaderReceived = false;
        processedBytesCount = 0;
        jpegSize = 0;
        m_imageBuffer.clear();

        // Parse JPEG size
        int beginIndex = responseStr.indexOf(':', index) + 2; // +2 - ": "
        if (beginIndex == -1) {
            qDebug() << "Separator ':' is not found. Drop packet";
            emit badFrameReceived();
            return;
        }
        int endIndex = responseStr.indexOf("\r\n", beginIndex);
        if (endIndex == -1) {
            qDebug() << "Separator '\\r\\n' is not found. Drop packet";
            emit badFrameReceived();
            return;
        }

        bool result = false;
        jpegSize = responseStr.mid(beginIndex, endIndex - beginIndex).toInt(&result);
        if (result == false) {
            qDebug() << "Cannot convert JPEG size from string. Drop packet";
            emit badFrameReceived();
            return;
        }

        // Remove HTTP header
        response = response.mid(endIndex + 4);
        isHeaderReceived = true;
    }

    if (isHeaderReceived == true) {

        // Buffering image
        processedBytesCount += response.size();
        m_imageBuffer.append(response);

        // Transfer complete - update image
        if (processedBytesCount >= jpegSize) {

            // Remove boundary part
            m_imageBuffer.resize(jpegSize);

            // Update image
            m_frameProvider->setImageRawData(m_imageBuffer);
            isHeaderReceived = false; // Wait new content header

            emit frameReceived();
        }
    }
}

void StreamService::httpConnectionClosed() {
    m_eventLoop->exit();
    emit connectionClosed();
}
