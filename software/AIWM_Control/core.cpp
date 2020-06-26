#include "core.h"
#include <QGuiApplication>
#include <QEventLoop>


Core::Core(StreamFrameProvider* streamFrameProvider, QObject *parent) :
    QObject(parent), m_streamService(streamFrameProvider), m_commandForSend(SWLP_CMD_NONE) {

	// Setup SWLP
	connect(this, &Core::swlpRunCommunication, &m_swlp, &Swlp::runCommunication, Qt::ConnectionType::QueuedConnection);
	connect(&m_swlp, &Swlp::statusPayloadReceived, this, &Core::swlpStatusPayloadProcess, Qt::ConnectionType::QueuedConnection);
	connect(&m_swlp, &Swlp::requestCommandPayload, this, &Core::swlpCommandPayloadPrepare, Qt::ConnectionType::QueuedConnection);
	m_swlp.moveToThread(&m_swlpThread);
    
    // Setup StreamService
    connect(this, &Core::streamServiceRun, &m_streamService, &StreamService::runService, Qt::ConnectionType::QueuedConnection);
    connect(&m_streamService, &StreamService::frameReceived, this,
            [this](void) { emit streamServiceFrameReceived(); }, Qt::ConnectionType::QueuedConnection);
    connect(&m_streamService, &StreamService::badFrameReceived, this,
            [this](void) { emit streamServiceBadFrameReceived(); }, Qt::ConnectionType::QueuedConnection);
    connect(&m_streamService, &StreamService::connectionClosed, this,
            [this](void) { emit streamServiceConnectionClosed(); }, Qt::ConnectionType::QueuedConnection);
    m_streamService.moveToThread(&m_streamServiceThread);
}

Core::~Core() {
	m_swlpThread.exit();
    m_streamServiceThread.exit();
	m_swlpThread.wait();
    m_streamServiceThread.wait();
}

void Core::runCommunication() {
	m_commandForSend = SWLP_CMD_NONE;
	m_swlpThread.start();
	emit swlpRunCommunication();
}

void Core::stopCommunication() {
	m_swlpThread.quit();
}

void Core::runStreamService() {
    m_streamServiceThread.start();
    emit streamServiceRun();
}

void Core::stopStreamService() {
    m_streamServiceThread.quit();
}

void Core::sendGetUpCommand()           { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_UP; 					 }
void Core::sendGetDownCommand()         { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DOWN; 				 }
void Core::sendUpDownCommand()          { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_UP_DOWN; 				 }
void Core::sendPushPullCommand()        { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_PUSH_PULL; 		     }
void Core::sendAttackLeftCommand()      { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ATTACK_LEFT; 			 }
void Core::sendAttackRightCommand()     { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ATTACK_RIGHT; 		 }
void Core::sendDanceCommand()			{ m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DANCE;				 }
void Core::sendRotateXCommand()         { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ROTATE_X;				 }
void Core::sendRotateZCommand()			{ m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ROTATE_Z;				 }
void Core::sendStopMoveCommand()        { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_NONE;					 }
void Core::sendStartMotionCommand(QVariant stepLength, QVariant curvature) {
    int16_t stepLengthInt16 = stepLength.toInt();
    m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DIRECT;
    if (stepLengthInt16 < 0) {
        m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_REVERSE;
    }

    m_stepLenght = abs(stepLengthInt16);
    m_curvature = curvature.toInt();
}


//
// SLOTS
//
void Core::swlpStatusPayloadProcess(const swlp_status_payload_t* payload) {
	emit frameReceived();
	emit systemStatusUpdated(payload->system_status);
	emit moduleStatusUpdated(payload->module_status);
	emit voltageValuesUpdated(payload->battery_voltage);
	emit batteryChargeUpdated(payload->battery_charge);
}

void Core::swlpCommandPayloadPrepare(swlp_command_payload_t* payload) {
	payload->command = m_commandForSend;
	payload->step_length = m_stepLenght;
	payload->curvature = m_curvature;
}
