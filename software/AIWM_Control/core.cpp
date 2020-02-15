#include "core.h"
#include <QGuiApplication>
#include <QDebug>
#include <QEventLoop>


Core::Core(QObject *parent) : QObject(parent) {

	m_statusUpdateTimer.setInterval(1000);
	connect(&m_statusUpdateTimer, &QTimer::timeout, this, &Core::statusUpdateTimer);

	//m_wirelessModbus = new WirelessModbus;
	//m_wirelessModbus->moveToThread(&m_thread);
	//m_thread.start();
	//
	//connect(this, &Core::connectToServerSignal, m_wirelessModbus, &WirelessModbus::connectToServer);
	//connect(this, &Core::disconnectFromServerSignal, m_wirelessModbus, &WirelessModbus::disconnectFromServer);
	//connect(this, &Core::writeDataToRamSignal, m_wirelessModbus, &WirelessModbus::writeRAM);
	//connect(this, &Core::readDataFromRamSignal, m_wirelessModbus, &WirelessModbus::readRAM);
}

Core::~Core() {

	m_thread.quit();
	//delete m_wirelessModbus;
}

bool Core::connectToServer() {

	/*emit connectToServerSignal();
	this->waitOperationCompleted();

	if (m_wirelessModbus->operationResult() == false) {
		m_statusUpdateTimer.stop();
		return false;
	}

	m_statusUpdateTimer.start();*/
	return true;
}

void Core::disconnectFromServer() {

	m_statusUpdateTimer.stop();

	emit disconnectFromServerSignal();
	this->waitOperationCompleted();
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

bool Core::sendEnableFrontDistanceSensorCommand()  { return 0; }//return writeToSCR(SCR_CMD_ENABLE_FRONT_SENSOR, 3);             }
bool Core::sendDisableFrontDistanceSensorCommand() { return 0; }//return writeToSCR(SCR_CMD_DISABLE_FRONT_SENSOR, 3);            }


//
// PROTECTED
//
bool Core::writeToSCR(int cmd, int retryCount) {

	/*QByteArray data;
	data.push_back(static_cast<char>(cmd));

	for (int i = 0; i < retryCount; ++i) {

		emit writeDataToRamSignal(SCR_REGISTER_ADDRESS, data);
		this->waitOperationCompleted();

		if (m_wirelessModbus->operationResult()) {
			return true;
		}
	}*/

	return false;
}

void Core::waitOperationCompleted() {

	/*while (m_wirelessModbus->isOperationInProgress() == false);
	while (m_wirelessModbus->isOperationInProgress() == true) {
		QGuiApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	}*/
}



//
// SLOTS
//
void Core::statusUpdateTimer() {

	/*QByteArray buffer;
	emit readDataFromRamSignal(MAIN_BLOCK_ADDRESS, &buffer, 10);
	waitOperationCompleted();

	if (m_wirelessModbus->operationResult() == false) {
		return;
	}

	// Make error status
	if (buffer.size() == 10) {
		uint16_t errorStatus = static_cast<uint16_t>((buffer[1] << 8) | (buffer[0] << 0));
		emit systemStatusUpdatedSignal(errorStatus);
		emit systemVoltageUpdatedSignal(buffer[2], buffer[3], buffer[4]);
	}*/
}
