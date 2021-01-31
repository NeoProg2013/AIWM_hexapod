#ifndef SWLP_H
#define SWLP_H
#include <QObject>
#include <QUdpSocket>
#include <QEventLoop>
#include <QThread>
#include <QTimer>
#include <atomic>
#include "swlp_protocol.h"


class Swlp : public QThread {
    Q_OBJECT
public:
    Swlp() : QThread(nullptr) {}
    virtual ~Swlp() {}
    virtual bool startThread(void* core);
    virtual void stopThread();

signals:
    void connectionClosed();

protected:
    virtual void run() override;
    virtual uint16_t calculateCRC16(const uint8_t* frameByteArray, int size);

protected slots:
    virtual void datagramReceivedEvent();
    virtual void sendCommandPayloadEvent();

protected:
    std::atomic<bool> m_isReady     {false};
    std::atomic<bool> m_isError     {false};

    QUdpSocket* m_socket            {nullptr};
    QTimer* m_timeoutTimer          {nullptr};
    QEventLoop* m_eventLoop         {nullptr};
    void* m_core                    {nullptr};
};


#endif // SWLP_H
