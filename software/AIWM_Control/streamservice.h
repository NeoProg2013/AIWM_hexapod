#ifndef STREAM_SERVICE_H
#define STREAM_SERVICE_H
#include <QObject>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include "streamframeprovider.h"


class StreamService : public QThread {
    Q_OBJECT
public:
    StreamService(StreamFrameProvider* frameProvider) : QThread(nullptr), m_frameProvider(frameProvider) {}
    virtual ~StreamService() {}

    Q_INVOKABLE bool startService();
    Q_INVOKABLE void stopService();

    virtual void setIpAddress(QString cameraIp);

signals:
    void ipAddressUpdated(QVariant ipAddress);
    void frameReceived();
    void connectionClosed();

protected:
    virtual void run() override;

protected slots:
    virtual void httpDataReceived();

protected:
    std::atomic<bool> m_isReady                 {false};
    std::atomic<bool> m_isError                 {false};

    QEventLoop* m_eventLoop                     {nullptr};
    QTimer* m_timeoutTimer                      {nullptr};
    QNetworkReply* m_requestReply               {nullptr};

    QString m_cameraIp                          {"255.255.255.255"};
    QByteArray m_buffer;
    StreamFrameProvider* const m_frameProvider  {nullptr};
};

#endif // STREAM_SERVICE_H
