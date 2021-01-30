#ifndef SWLP_H
#define SWLP_H

#include <QObject>
#include <QUdpSocket>
#include <QEventLoop>
#include <QThread>
#include "swlp_protocol.h"

#include <atomic>


class Swlp : public QObject {
    Q_OBJECT
public:
    Swlp() : QObject(nullptr) {}
    virtual ~Swlp() {}
    bool start(void* core);
    void stop();

protected:
    void threadRun();

protected:
    void datagramReceivedEvent();       // SLOT
    void sendCommandPayloadEvent();     // SLOT
    uint16_t calculateCRC16(const uint8_t* frameByteArray, int size);

protected:
    QThread* m_thread               {nullptr};
    QUdpSocket* m_socket            {nullptr};
    QEventLoop* m_eventLoop         {nullptr};
    void* m_core                    {nullptr};
    std::atomic<bool> m_isStarted;
    std::atomic<bool> m_isError;
};


#endif // SWLP_H
