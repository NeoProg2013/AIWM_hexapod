#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QThread>
#include "swlp.h"
#include "streamservice.h"
#include "streamframeprovider.h"

class Core : public QObject
{
    Q_OBJECT
public:
    explicit Core(StreamFrameProvider* streamFrameProvider, QObject *parent = nullptr);
    virtual ~Core();

    Q_INVOKABLE void runCommunication();
    Q_INVOKABLE void stopCommunication();

    Q_INVOKABLE void runStreamService();
    Q_INVOKABLE void stopStreamService();

    Q_INVOKABLE void sendGetUpCommand();
    Q_INVOKABLE void sendGetDownCommand();
    Q_INVOKABLE void sendUpDownCommand();
    Q_INVOKABLE void sendPushPullCommand();
    Q_INVOKABLE void sendAttackLeftCommand();
    Q_INVOKABLE void sendAttackRightCommand();
    Q_INVOKABLE void sendDanceCommand();
    Q_INVOKABLE void sendRotateXCommand();
    Q_INVOKABLE void sendRotateZCommand();
    Q_INVOKABLE void sendStopMoveCommand();
    Q_INVOKABLE void sendStartMotionCommand(QVariant stepLength, QVariant curvature);

signals:
    // To SWLP module
    void swlpRunCommunication();
    
    // To StreamService module
    void streamServiceRun();

    // To QML
    void frameReceived();
    void systemStatusUpdated(QVariant newSystemStatus);
    void moduleStatusUpdated(QVariant newModuleStatus);
    void voltageValuesUpdated(QVariant newBatteryVoltage);
    void batteryChargeUpdated(QVariant newBatteryCharge);

    // To QML from StreamService module
    void streamServiceFrameReceived();
    void streamServiceBadFrameReceived();
    void streamServiceConnectionClosed();

public slots:
    // From SWLP module
    void swlpStatusPayloadProcess(const swlp_status_payload_t* payload);
    void swlpCommandPayloadPrepare(swlp_command_payload_t* payload);

protected:
    Swlp m_swlp;
    StreamService m_streamService;

    QThread m_streamServiceThread;
    QThread m_swlpThread;

    uint8_t m_commandForSend;
    uint8_t m_stepLenght;
    int16_t m_curvature;
};

#endif // CORE_H
