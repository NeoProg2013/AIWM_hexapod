#ifndef SWLP_H
#define SWLP_H
#include <QObject>
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

    Q_INVOKABLE void sendGetUpCommand()    { setCommand(SWLP_CMD_SELECT_SCRIPT_UP);          }
    Q_INVOKABLE void sendGetDownCommand()  { setCommand(SWLP_CMD_SELECT_SCRIPT_DOWN);        }
    Q_INVOKABLE void sendXRotateCommand()  { setCommand(SWLP_CMD_SELECT_SCRIPT_X_ROTATE);    }
    Q_INVOKABLE void sendZRotateCommand()  { setCommand(SWLP_CMD_SELECT_SCRIPT_Z_ROTATE);    }
    Q_INVOKABLE void sendXYRotateCommand() { setCommand(SWLP_CMD_SELECT_SCRIPT_XY_ROTATE);   }
    Q_INVOKABLE void sendUpDownCommand()   { setCommand(SWLP_CMD_SELECT_SCRIPT_UP_DOWN);     }
    Q_INVOKABLE void sendPushPullCommand() { setCommand(SWLP_CMD_SELECT_SCRIPT_Z_PUSH_PULL); }
    Q_INVOKABLE void sendXSwayCommand()    { setCommand(SWLP_CMD_SELECT_SCRIPT_X_SWAY);      }
    Q_INVOKABLE void sendSquareCommand()   { setCommand(SWLP_CMD_SELECT_SCRIPT_SQUARE);      }

    Q_INVOKABLE void sendStartMotionCommand(QVariant speed, QVariant distance, QVariant curvature, QVariant stepHeight,
                                            QVariant surfacePointX, QVariant surfacePointY, QVariant surfacePointZ,
                                            QVariant surfaceRotateX, QVariant surfaceRotateY, QVariant surfaceRotateZ);
    Q_INVOKABLE void sendStopMoveCommand(QVariant surfacePointX, QVariant surfacePointY, QVariant surfacePointZ,
                                         QVariant surfaceRotateX, QVariant surfaceRotateY, QVariant surfaceRotateZ);
signals:
    void frameReceived();
    void systemStatusUpdated(QVariant newSystemStatus, QVariant newModuleStatus);
    void batteryStatusUpdated(QVariant newBatteryCharge, QVariant newBatteryVoltage);
    void connectionClosed();

protected:
    virtual void run() override;
    virtual void setCommand(uint8_t command);
    virtual uint16_t calculateCRC16(const uint8_t* frameByteArray, int size);

protected slots:
    virtual void datagramReceivedEvent();
    virtual void sendCommandPayloadEvent();

protected:
    std::atomic<bool> m_isReady     {false};
    std::atomic<bool> m_isError     {false};

    QMutex m_stopServiceMutex;
    QUdpSocket* m_socket            {nullptr};
    QTimer* m_timeoutTimer          {nullptr};

    QMutex m_eventLoopMutex;
    QEventLoop* m_eventLoop         {nullptr};

    QMutex m_payloadMutex;
    swlp_command_payload_t m_commandPayload;
};


#endif // SWLP_H
