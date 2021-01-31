#ifndef STREAM_SERVICE_H
#define STREAM_SERVICE_H
#include <QObject>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include "streamframeprovider.h"


class StreamService : public QObject {
    Q_OBJECT
public:
    StreamService(StreamFrameProvider* frameProvider) : QObject(nullptr), m_frameProvider(frameProvider) {}
    virtual ~StreamService() {}
    bool start(QString cameraIp);
    void stop();

signals:
    void frameReceived();
    void badFrameReceived();
    void connectionClosed();

protected:
    void threadRun();

protected slots:
    void httpDataReceived();

protected:
    QThread* m_thread                           {nullptr};
    std::atomic<bool> m_isStarted;
    std::atomic<bool> m_isError;

    QEventLoop* m_eventLoop                     {nullptr};
    QTimer* m_timeoutTimer                      {nullptr};
    QNetworkReply* m_requestReply               {nullptr};

    QString m_cameraIp;
    QByteArray m_buffer;
    StreamFrameProvider* const m_frameProvider  {nullptr};
};

#endif // STREAM_SERVICE_H
