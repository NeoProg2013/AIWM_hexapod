#ifndef SWLP_H
#define SWLP_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QEventLoop>
#include "swlp_protocol.h"


class Swlp : public QObject
{
    Q_OBJECT
public:
    explicit Swlp(QObject* parent = nullptr);
    virtual ~Swlp();

public slots:
    void runCommunication();

signals:
    void requestCommandPayload(swlp_command_payload_t* payload);
    void statusPayloadReceived(const swlp_status_payload_t* payload);


protected slots:
    void datagramReceivedEvent();
    void sendCommandPayloadEvent();

protected:
    uint16_t calculateCRC16(const uint8_t *frameByteArray, int size);

private:
    bool m_isRunning                            {false};
    QEventLoop* m_eventLoop                     {nullptr};
    QUdpSocket* m_socket                        {nullptr};
    QTimer* m_sendTimer                         {nullptr};
    swlp_command_payload_t m_commandPayload;
    swlp_status_payload_t m_statusPayload;
};

#endif // SWLP_H
