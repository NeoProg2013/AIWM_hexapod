#include "streamframeprovider.h"

StreamFrameProvider::StreamFrameProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap),
      m_imageWidth(640), m_imageHeight(480),
      m_lastPixmap(m_imageWidth, m_imageHeight)
{
}

StreamFrameProvider::~StreamFrameProvider() {}

QPixmap StreamFrameProvider::requestPixmap(const QString&, QSize *size, const QSize&)
{
    if (size) {
        *size = QSize(m_imageWidth, m_imageHeight);
    }
    return m_lastPixmap;
}

void StreamFrameProvider::setImageRawData(const QByteArray& rawData) {
    QPixmap pixmap(m_imageWidth, m_imageHeight);
    if (pixmap.loadFromData(rawData) == true) {
        m_lastPixmap = pixmap;
    }
}
