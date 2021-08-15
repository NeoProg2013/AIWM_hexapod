#ifndef STREAMFRAMEPROVIDER_H
#define STREAMFRAMEPROVIDER_H

#include <QQuickImageProvider>
#include <QByteArray>
#include <QFile>
#include <QMutex>


class StreamFrameProvider : public QQuickImageProvider
{
public:
    StreamFrameProvider();
    virtual ~StreamFrameProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
    void setImageRawData(const QByteArray& rawData);

private:
    const int m_imageWidth;
    const int m_imageHeight;
    QMutex m_mutex;
    QPixmap m_lastPixmap;
    QFile file;
};

#endif // STREAMFRAMEPROVIDER_H
