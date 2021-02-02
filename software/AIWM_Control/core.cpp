#include "core.h"
#include <QGuiApplication>
#include <QEventLoop>


Core::Core(StreamService* streamService) : QObject(nullptr), m_streamService(streamService) {
    connect(&m_swlp, &Swlp::connectionClosed, this, [this](void) { emit swlpConnectionClosed(); }, Qt::ConnectionType::QueuedConnection);
}

bool Core::startCommunication() {
    m_commandForSend = SWLP_CMD_NONE;
    return m_swlp.startThread(this);
}
void Core::stopCommunication() {
    m_swlp.stopThread();
}

void Core::sendGetUpCommand()       { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_UP;           }
void Core::sendGetDownCommand()     { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DOWN;         }
void Core::sendUpDownCommand()      { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_UP_DOWN;      }
void Core::sendPushPullCommand()    { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_PUSH_PULL;    }
void Core::sendAttackLeftCommand()  { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ATTACK_LEFT;  }
void Core::sendAttackRightCommand() { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ATTACK_RIGHT; }
void Core::sendDanceCommand()       { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DANCE;        }
void Core::sendRotateXCommand()     { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ROTATE_X;     }
void Core::sendRotateZCommand()     { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ROTATE_Z;     }
void Core::sendStopMoveCommand()    { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_NONE;         }
void Core::sendStartMotionCommand(QVariant stepLength, QVariant curvature) {
    int16_t stepLengthInt16 = stepLength.toInt();
    m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DIRECT;
    if (stepLengthInt16 < 0) {
        m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_REVERSE;
    }

    m_stepLenght = abs(stepLengthInt16);
    m_curvature = curvature.toInt();
}
void Core::setMotionSpeed(QVariant motionSpeed) {
    m_motionSpeed = motionSpeed.toInt();
}



void Core::swlpStatusPayloadProcess(const swlp_status_payload_t* payload) {
    emit swlpFrameReceived();
    emit swlpStatusUpdated(payload->system_status, payload->module_status);
    emit swlpVoltageValuesUpdated(payload->battery_charge, payload->battery_voltage);

    QByteArray ipAddress(reinterpret_cast<const char*>(payload->camera_ip));
    m_streamService->setIpAddress(ipAddress);
}
void Core::swlpCommandPayloadPrepare(swlp_command_payload_t* payload) {
    payload->command = m_commandForSend;
    payload->step_length = m_stepLenght;
    payload->curvature = m_curvature;
    payload->motion_speed = m_motionSpeed;
}
