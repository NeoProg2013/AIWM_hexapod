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
    virtual bool startThread(QString cameraIp); // Call from core thread
    virtual void stopThread();                  // Call from core thread

signals:
    void frameReceived();
    void badFrameReceived();
    void connectionClosed();

protected:
    virtual void run() override;

protected slots:
    virtual void httpDataReceived();

protected:
    std::atomic<bool> m_isReady;
    std::atomic<bool> m_isError;

    QEventLoop* m_eventLoop                     {nullptr};
    QTimer* m_timeoutTimer                      {nullptr};
    QNetworkReply* m_requestReply               {nullptr};

    QString m_cameraIp;
    QByteArray m_buffer;
    StreamFrameProvider* const m_frameProvider  {nullptr};
};

#endif // STREAM_SERVICE_H
