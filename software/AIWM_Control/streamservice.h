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
    virtual void runService();

signals:
    void frameReceived();
    void badFrameReceived();
    void connectionClosed();

public slots:
    void httpDataReceived();
    void httpConnectionClosed();

private:
    bool m_isRunning;
    QEventLoop* m_eventLoop;

    QNetworkAccessManager* m_manager;
    QNetworkReply* m_requestReply;
    QTimer* m_timeoutTimer;

    QByteArray m_imageBuffer;

    StreamFrameProvider* m_frameProvider;
};

#endif // TCPCLIENT_H