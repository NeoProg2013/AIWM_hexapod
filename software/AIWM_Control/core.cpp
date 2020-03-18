#include "core.h"
#include <QGuiApplication>
#include <QEventLoop>


Core::Core(QObject *parent) : QObject(parent), m_commandForSend(SWLP_CMD_NONE) {

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
	m_commandForSend = SWLP_CMD_NONE;
	m_swlpThread.start();
	emit swlpRunCommunication();
}

void Core::stopCommunication() {
	m_swlpThread.quit();
}

void Core::sendGetUpCommand()           { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_UP; 					 }
void Core::sendGetDownCommand()         { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DOWN; 				 }
void Core::sendStopMoveCommand()        { m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_NONE;					 }
void Core::sendStartMotionCommand(QVariant stepLength, QVariant curvature) {

    int16_t stepLengthInt16 = stepLength.toInt();
    m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_DIRECT;
    if (stepLengthInt16 < 0) {
        m_commandForSend = SWLP_CMD_SELECT_SEQUENCE_REVERSE;
    }

    m_stepLenght = abs(stepLengthInt16);
    m_curvature = curvature.toInt();

    qDebug() << m_stepLenght << " " << m_curvature;
}


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
