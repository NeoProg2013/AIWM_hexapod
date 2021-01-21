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

    Q_INVOKABLE void setMotionSpeed(QVariant motionSpeed);

signals:
    // To SWLP module
    void swlpRunCommunication();
    
    // To StreamService module
    void streamServiceRun(QString cameraIp);

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
    void streamServiceIpAddressUpdate(QVariant ipAddress);

public slots:
    // From SWLP module
    void swlpStatusPayloadProcess(const swlp_status_payload_t* payload);
    void swlpCommandPayloadPrepare(swlp_command_payload_t* payload);

protected:
    Swlp m_swlp;
    StreamService m_streamService;

    QThread m_streamServiceThread;
    QThread m_swlpThread;

    uint8_t m_commandForSend        {SWLP_CMD_SELECT_SEQUENCE_NONE};
    uint8_t m_stepLenght            {0};
    int16_t m_curvature             {0};
    uint8_t m_motionSpeed           {90}; // %

    QString m_cameraIp              {"255.255.255.255"};
};

#endif // CORE_H
