#ifndef SWLP_H
#define SWLP_H
#include <QObject>
#include <QUdpSocket>
#include <QEventLoop>
#include <QThread>
#include <atomic>
#include "swlp_protocol.h"


class Swlp : public QObject {
    Q_OBJECT
public:
    Swlp() : QObject(nullptr) {}
    virtual ~Swlp() {}
    bool start(void* core);
    void stop();

protected:
    void threadRun();
    uint16_t calculateCRC16(const uint8_t* frameByteArray, int size);

protected slots:
    void datagramReceivedEvent();       // SLOT
    void sendCommandPayloadEvent();     // SLOT

protected:
    QThread* m_thread               {nullptr};
    std::atomic<bool> m_isStarted;
    std::atomic<bool> m_isError;

    QUdpSocket* m_socket            {nullptr};
    QEventLoop* m_eventLoop         {nullptr};
    void* m_core                    {nullptr};
};


#endif // SWLP_H
