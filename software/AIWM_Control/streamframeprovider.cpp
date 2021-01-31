#include "streamframeprovider.h"
#include <QThread>

StreamFrameProvider::StreamFrameProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap),
      m_imageWidth(480), m_imageHeight(480),
      m_lastPixmap(m_imageWidth, m_imageHeight)
{
}

StreamFrameProvider::~StreamFrameProvider() {}

QPixmap StreamFrameProvider::requestPixmap(const QString&, QSize *size, const QSize&) {
    //qDebug() << "[StreamFrameProvider]" << QThread::currentThreadId() << "call requestPixmap()";
    if (size) {
        *size = QSize(m_imageWidth, m_imageHeight);
    }

    m_mutex.lock();
    QPixmap pixmap(m_lastPixmap);
    m_mutex.unlock();
    return pixmap;
}

void StreamFrameProvider::setImageRawData(const QByteArray& rawData) {
    //qDebug() << "[StreamFrameProvider]" << QThread::currentThreadId() << "call setImageRawData()";
    QPixmap pixmap(m_imageWidth, m_imageHeight);
    if (pixmap.loadFromData(rawData, "JPEG") == true) {
        m_mutex.lock();
        m_lastPixmap = pixmap.copy((640 - m_imageWidth) / 2, 0, m_imageWidth, m_imageHeight);
        m_mutex.unlock();
    } else {
        qDebug() << "Can't loadFromData: " << rawData.size() << " " << (int)rawData[0] << (int)rawData[1];
    }
}
