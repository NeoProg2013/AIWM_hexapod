#ifndef SWLP_H
#define SWLP_H
#include <QObject>
#include <QVariant>
#include <QUdpSocket>
#include <QEventLoop>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <atomic>
#include "swlp_protocol.h"


class Swlp : public QThread {
    Q_OBJECT
public:
    Swlp() : QThread(nullptr) {}
    virtual ~Swlp() {}

    //
    // Q_INVOKABLE methods call from GUI thread
    //
    Q_INVOKABLE bool startService();
    Q_INVOKABLE void stopService();

    Q_INVOKABLE void sendMotionCommand(QVariant speed, QVariant distance, QVariant curvature, QVariant stepHeight,
                                       QVariant px, QVariant py, QVariant pz, QVariant rx, QVariant ry, QVariant rz,
                                       QVariant isStabEnabled);

signals:
    void frameReceived();
    void systemStatusUpdated(QVariant newSystemStatus, QVariant newModuleStatus);
    void batteryStatusUpdated(QVariant newBatteryCharge, QVariant newBatteryVoltage);
    void surfaceParametersUpdated(QVariant px, QVariant py, QVariant pz,
                                  QVariant rx, QVariant ry, QVariant rz);
    void connectionClosed();

protected:
    virtual void run() override;
    virtual uint16_t calculateCRC16(const uint8_t* frameByteArray, int size);

protected slots:
    virtual void datagramReceivedEvent();
    virtual void sendCommandPayloadEvent();

protected:
    std::atomic<bool> m_isReady                 {false};
    std::atomic<bool> m_isError                 {false};

    QMutex m_stopServiceMutex;
    QUdpSocket* m_socket                        {nullptr};
    QTimer* m_timeoutTimer                      {nullptr};

    QMutex m_eventLoopMutex;
    QEventLoop* m_eventLoop                     {nullptr};

    QMutex m_requestMutex;
    swlp_request_t m_swlpRequst;
};


#endif // SWLP_H
