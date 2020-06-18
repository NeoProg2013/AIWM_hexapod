#ifndef STREAMFRAMEPROVIDER_H
#define STREAMFRAMEPROVIDER_H

#include <QQuickImageProvider>
#include <QByteArray>


class StreamFrameProvider : public QQuickImageProvider
{
public:
    StreamFrameProvider();
    virtual ~StreamFrameProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
    virtual void setImageRawData(const QByteArray& rawData);

private:
    QByteArray m_imageRawData;
};

#endif // STREAMFRAMEPROVIDER_H
