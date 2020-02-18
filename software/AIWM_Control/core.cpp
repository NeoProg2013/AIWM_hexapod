#include "core.h"
#include <QGuiApplication>
#include <QEventLoop>


Core::Core(QObject *parent) : QObject(parent) {

	m_commandForSend = SWLP_CMD_NONE;

	// Setup SWLP
	connect(this, &Core::swlpRunCommunication, &m_swlp, &Swlp::runCommunication, Qt::ConnectionType::QueuedConnection);
	connect(&m_swlp, &Swlp::statusPayloadReceived, this, &Core::swlpStatusPayloadProcess, Qt::ConnectionType::QueuedConnection);
	connect(&m_swlp, &Swlp::requestCommandPayload, this, &Core::swlpCommandPayloadPrepare, Qt::ConnectionType::QueuedConnection);
	m_swlp.moveToThread(&m_swlpThread);
}

Core::~Core() {
	m_swlpThread.exit();
	m_swlpThread.wait();
}

void Core::runCommunication() {
	m_swlpThread.start();
	emit swlpRunCommunication();
}

void Core::stopCommunication() {
	m_swlpThread.exit();
}

void Core::sendGetUpCommand()           { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_UP; 					 }
void Core::sendGetDownCommand()         { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DOWN; 				 }
void Core::sendRunCommand()				{ m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_RUN; 					 }
void Core::sendDirectMoveCommand()      { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DIRECT_MOVEMENT; 		 }
void Core::sendReverseMoveCommand()     { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_REVERSE_MOVEMENT; 	 }
void Core::sendRotateLeftCommand()      { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ROTATE_LEFT; 			 }
void Core::sendRotateRightCommand()     { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ROTATE_RIGHT; 		 }
void Core::sendDirectMoveSlowCommand()  { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DIRECT_MOVEMENT_SLOW;  }
void Core::sendReverseMoveSlowCommand() { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_REVERSE_MOVEMENT_SLOW; }
void Core::sendShiftLeftCommand()       { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_SHIFT_LEFT; 			 }
void Core::sendShiftRightCommand()      { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_SHIFT_RIGHT; 			 }
void Core::sendAttackLeftCommand()      { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ATTACK_LEFT; 			 }
void Core::sendAttackRightCommand()     { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ATTACK_RIGHT; 		 }
void Core::sendDanceCommand()           { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DANCE; 				 }
void Core::sendRotateXCommand()         { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ROTATE_X; 			 }
void Core::sendRotateZCommand()         { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_ROTATE_Z; 			 }
void Core::sendStopMoveCommand()        { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_NONE;					 }
void Core::sendSwitchLightCommand()     { m_commandForSend = SWLP_CMD_SWITCH_LIGHT;					         }



//
// SLOTS
//
void Core::swlpStatusPayloadProcess(const swlp_status_payload_t* payload) {
	emit frameReceived();
	emit systemStatusUpdated(payload->system_status);
	emit moduleStatusUpdated(payload->module_status);
	emit voltageValuesUpdated(payload->battery_cell_voltage[0], payload->battery_cell_voltage[1], payload->battery_cell_voltage[2], payload->battery_voltage);
	emit batteryChargeUpdated(payload->battery_charge);
}

void Core::swlpCommandPayloadPrepare(swlp_command_payload_t* payload) {
	payload->command = m_commandForSend;
}
