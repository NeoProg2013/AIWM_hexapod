#include "streamframeprovider.h"

StreamFrameProvider::StreamFrameProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

StreamFrameProvider::~StreamFrameProvider() {}

QPixmap StreamFrameProvider::requestPixmap(const QString&, QSize *size, const QSize&)
{
    int width = 640;
    int height = 480;
    if (size) {
        *size = QSize(width, height);
    }

    QPixmap pixmap(width, height);
    if (m_imageRawData.size() != 0) {

        if (pixmap.loadFromData(m_imageRawData) == false) {
            pixmap = QPixmap(width, height);
            pixmap.fill(QColor(0, 0, 0));
        }
    }
    else {
        pixmap.fill(QColor(0, 0, 0));
    }
    return pixmap;
}

void StreamFrameProvider::setImageRawData(const QByteArray& rawData) {
    m_imageRawData = rawData;
}
