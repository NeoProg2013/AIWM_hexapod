#include "core.h"
#include <QGuiApplication>
#include <QDebug>
#include <QEventLoop>


Core::Core(QObject *parent) : QObject(parent) {
	qDebug() << "Core::Core(). Thread id =" << QThread::currentThreadId();

	// Setup SWLP
	connect(this, &Core::swlpRunCommunication, &m_swlp, &Swlp::runCommunication, Qt::ConnectionType::QueuedConnection);
	m_swlp.moveToThread(&m_swlpThread);
}

Core::~Core() {
	qDebug() << "Core::~Core(). Thread id =" << QThread::currentThreadId();

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

void Core::sendGetUpCommand()                      { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_UP, 1);                    */ }
void Core::sendGetDownCommand()                    { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_DOWN, 1);                  */ }
void Core::sendRunCommand()						   { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_RUN, 1);                   */ }
void Core::sendDirectMoveCommand()                 { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_DIRECT_MOVEMENT, 1);       */ }
void Core::sendReverseMoveCommand()                { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_REVERSE_MOVEMENT, 1);      */ }
void Core::sendRotateLeftCommand()                 { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_ROTATE_LEFT, 1);           */ }
void Core::sendRotateRightCommand()                { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_ROTATE_RIGHT, 1);          */ }
void Core::sendDirectMoveSlowCommand()             { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_DIRECT_MOVEMENT_SLOW, 1);  */ }
void Core::sendReverseMoveSlowCommand()            { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_REVERSE_MOVEMENT_SLOW, 1); */ }
void Core::sendShiftLeftCommand()                  { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_SHIFT_LEFT, 1);            */ }
void Core::sendShiftRightCommand()                 { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_SHIFT_RIGHT, 1);           */ }
													 /*																 */
void Core::sendAttackLeftCommand()                 { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_ATTACK_LEFT, 1);           */ }
void Core::sendAttackRightCommand()                { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_ATTACK_RIGHT, 1);          */ }
void Core::sendDanceCommand()                      { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_DANCE, 1);                 */ }
void Core::sendRotateXCommand()                    { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_ROTATE_X, 1);              */ }
void Core::sendRotateZCommand()                    { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_ROTATE_Z, 1);              */ }
													 /*																 */
void Core::sendStopMoveCommand()                   { /*writeToSCR(SCR_CMD_SELECT_SEQUENCE_NONE, 5);                  */ }



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

	payload->command = 0xAA;
}
