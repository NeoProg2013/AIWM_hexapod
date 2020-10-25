#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include "streamframeprovider.h"


class StreamService : public QObject
{
    Q_OBJECT
public:
    explicit StreamService(StreamFrameProvider* frameProvider, QObject *parent = nullptr);
    virtual ~StreamService();

signals:
    void frameReceived();
    void badFrameReceived();
    void connectionClosed();

public slots:
    virtual void runService(QString cameraIp);
    void httpDataReceived();
    void httpConnectionClosed();

private:
    bool m_isRunning                        {false};
    QEventLoop* m_eventLoop                 {nullptr};
    QNetworkAccessManager* m_manager        {nullptr};
    QNetworkReply* m_requestReply           {nullptr};
    QTimer* m_timeoutTimer                  {nullptr};
    StreamFrameProvider* m_frameProvider    {nullptr};
    QByteArray m_imageBuffer;
};

#endif // TCPCLIENT_H
