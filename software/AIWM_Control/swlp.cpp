#include <QGuiApplication>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QDebug>
#include <QThread>
#include "swlp.h"
#define SERVER_IP_ADDRESS					("111.111.111.111")
#define SERVER_PORT							(3333)


Swlp::Swlp(QObject* parent) : QObject(parent) {
	qDebug() << "Swlp::Swlp()";

	m_socket = nullptr;
	m_sendTimer = nullptr;
	m_isRunning = false;

	// Clear payloads
	memset(&m_commandPayload, 0, sizeof(m_commandPayload));
	memset(&m_statusPayload, 0, sizeof(m_statusPayload));
}

Swlp::~Swlp() {
	qDebug() << "Swlp::~Swlp(). Thread id =" << QThread::currentThreadId();

	if (m_socket != nullptr) {
		m_sendTimer->stop();
		delete m_sendTimer;
	}

	if (m_socket != nullptr) {
		delete m_socket;
	}
}

void Swlp::runCommunication() {
	qDebug() << "Swlp::start(). Thread id =" << QThread::currentThreadId();
	if (m_isRunning == true) {
		return;
	}
	m_isRunning = true;

	// Setup UDP socket
	m_socket = new (std::nothrow) QUdpSocket();
	if (m_socket == nullptr) {
		qDebug() << "Cannot create QUdpSocket";
		return;
	}
	connect(m_socket, &QUdpSocket::readyRead, this, &Swlp::datagramReceivedEvent);
	if (m_socket->bind(SERVER_PORT) == false) {
		qDebug() << "Bind false";
		qDebug() << m_socket->errorString();
	}

	// Setup send command payload timer
	m_sendTimer = new (std::nothrow) QTimer();
	if (m_sendTimer == nullptr) {
		qDebug() << "Cannot create QTimer";
		return;
	}
	connect(m_sendTimer, &QTimer::timeout, this, &Swlp::sendCommandPayloadEvent);
	m_sendTimer->setSingleShot(false);
	m_sendTimer->setInterval(200);
	m_sendTimer->start();

	// Start event loop
	m_eventLoop = new QEventLoop;
	m_eventLoop->exec();
	qDebug() << "Exit from event loop";

	// Stop send command payload timer
	m_sendTimer->stop();

	m_isRunning = false;
	qDebug() << "Swlp::start() leaved";
}

void Swlp::datagramReceivedEvent() {
	qDebug() << "Swlp::datagramReceivedEvent(). Thread id =" << QThread::currentThreadId();

	swlp_frame_t swlp_frame;

	// Check datagram size
	qint64 datagram_size = m_socket->pendingDatagramSize();
	if (datagram_size != sizeof(swlp_frame_t)) {
		m_socket->readDatagram(reinterpret_cast<char*>(&swlp_frame), 0);
		qDebug() << "Wrong datagram size";
		return;
	}

	// Read frame
	m_socket->readDatagram(reinterpret_cast<char*>(&swlp_frame), sizeof(swlp_frame));

	// Verify SWLP frame
	if (swlp_frame.start_mark != SWLP_START_MARK_VALUE) {
		qDebug() << "Wrong SWLP_START_MARK_VALUE";
		return;
	}
	if (this->calculateCRC16(reinterpret_cast<const uint8_t*>(&swlp_frame), sizeof(swlp_frame)) != 0) {
		qDebug() << "Wrong CRC16";
		return;
	}

	// Process status payload
	memcpy(&m_statusPayload, swlp_frame.payload, sizeof(m_statusPayload));
	emit statusPayloadReceived(&m_statusPayload);

	qDebug() << "Status payload received";
}

void Swlp::sendCommandPayloadEvent() {
	qDebug() << "Swlp::sendCommandPayloadEvent(). Thread id =" << QThread::currentThreadId();

	// Request command payload
	emit requestCommandPayload(&m_commandPayload);

	// Make SWLP frame
	swlp_frame_t frame;
	memset(&frame, 0, sizeof(frame));

	frame.start_mark = SWLP_START_MARK_VALUE;
	frame.frame_number = 1;
	memcpy(frame.payload, &m_commandPayload, sizeof(m_commandPayload));
	frame.crc16 = 0;

	// Calculate CRC16
	uint16_t crc = this->calculateCRC16(reinterpret_cast<const uint8_t*>(&frame), sizeof(frame) - 2);
	frame.crc16 = crc;

	// Send SWLP frame
	QNetworkDatagram datagram;
	datagram.setDestination(QHostAddress(SERVER_IP_ADDRESS), SERVER_PORT);
	datagram.setData(QByteArray(reinterpret_cast<const char*>(&frame), sizeof(frame)));
	m_socket->writeDatagram(datagram);
}

//
// PROTECTED
//
uint16_t Swlp::calculateCRC16(const uint8_t* frame, int size) {

	uint16_t crc16 = 0xFFFF;
	uint16_t data = 0;
	uint16_t k = 0;

	while (size--) {
		crc16 ^= *frame++;
		k = 8;
		while (k--) {
			data = crc16;
			crc16 >>= 1;
			if (data & 0x0001) {
				crc16 ^= SWLP_CRC16_POLYNOM;
			}
		}
	}
	return crc16;
}
