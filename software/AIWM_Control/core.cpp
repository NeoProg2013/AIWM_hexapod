#include "core.h"
#include <QGuiApplication>
#include <QEventLoop>

Core::Core(StreamFrameProvider* streamFrameProvider) : QObject(nullptr), m_streamService(streamFrameProvider) {
    connect(&m_streamService, &StreamService::frameReceived,    this, [this](void) { emit streamServiceFrameReceived();    }, Qt::ConnectionType::QueuedConnection);
    connect(&m_streamService, &StreamService::badFrameReceived, this, [this](void) { emit streamServiceBadFrameReceived(); }, Qt::ConnectionType::QueuedConnection);
    connect(&m_streamService, &StreamService::connectionClosed, this, [this](void) { emit streamServiceConnectionClosed(); }, Qt::ConnectionType::QueuedConnection);
}
Core::~Core() {
    stopCommunication();
}

bool Core::runCommunication() {
    m_commandForSend = SWLP_CMD_NONE;
    return m_swlp.start(this);
}
void Core::stopCommunication() {
    m_swlp.stop();
    m_streamService.stop();
}
bool Core::runStreamService() {
    return m_streamService.start(m_cameraIp);
}

void Core::sendGetUpCommand()           { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_UP;           }
void Core::sendGetDownCommand()         { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DOWN;         }
void Core::sendUpDownCommand()          { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_UP_DOWN;      }
void Core::sendPushPullCommand()        { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_PUSH_PULL;    }
void Core::sendAttackLeftCommand()      { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ATTACK_LEFT;  }
void Core::sendAttackRightCommand()     { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ATTACK_RIGHT; }
void Core::sendDanceCommand()           { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DANCE;        }
void Core::sendRotateXCommand()         { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ROTATE_X;     }
void Core::sendRotateZCommand()         { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ROTATE_Z;     }
void Core::sendStopMoveCommand()        { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_NONE;         }
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
    emit frameReceived();
    emit systemStatusUpdated(payload->system_status);
    emit moduleStatusUpdated(payload->module_status);
    emit voltageValuesUpdated(payload->battery_voltage);
    emit batteryChargeUpdated(payload->battery_charge);

    QByteArray ipAddress(reinterpret_cast<const char*>(payload->camera_ip));
    QString newCameraIp(ipAddress);
    if (newCameraIp != m_cameraIp) {
        m_streamService.stop();
    }

    m_cameraIp = QString(ipAddress);
    emit streamServiceIpAddressUpdate(m_cameraIp);
}
void Core::swlpCommandPayloadPrepare(swlp_command_payload_t* payload) {
    payload->command = m_commandForSend;
    payload->step_length = m_stepLenght;
    payload->curvature = m_curvature;
    payload->motion_speed = m_motionSpeed;
}
