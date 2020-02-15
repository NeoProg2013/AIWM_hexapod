#include <QGuiApplication>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QDebug>
#include "wireless.h"
#define SERVER_IP_ADDRESS					("111.111.111.111")
#define SERVER_PORT							(3333)


Wireless::Wireless(QObject* parent) : QThread(parent) {


	qDebug() << "Wireless::Wireless";

	connect(&m_socket, &QUdpSocket::readyRead, this, &Wireless::datagramReceivedEvent);
	if (m_socket.bind(SERVER_PORT) == false) {
		qDebug() << "Bind false";
		qDebug() << m_socket.errorString();
	}

	connect(&m_sendTimer, &QTimer::timeout, this, &Wireless::sendTimerEvent);
	m_sendTimer.setSingleShot(false);
	m_sendTimer.start(500);

	memset(&m_command_payload, 0, sizeof(m_command_payload));
}

void Wireless::run() {

	/*while (true) {

		if (m_socket.bytesAvailable() >= sizeof(swlp_frame_t)) {

			QByteArray frame = m_socket.readAll();

			// Check frame size
			if (frame.size() != sizeof(swlp_frame_t)) {
				continue;
			}

			swlp_status_payload_t*frame.data();

		}
	}*/
}

void Wireless::datagramReceivedEvent() {

	qDebug() << "[Wireless] Datagram received";

	swlp_frame_t swlp_frame;
	qint64 datagram_size = m_socket.pendingDatagramSize();
	if (datagram_size != sizeof(swlp_frame_t)) {
		m_socket.readDatagram(reinterpret_cast<char*>(&swlp_frame), 0);
		qDebug() << "[Wireless] Wrong datagram size";
		return;
	}

	// Read frame
	m_socket.readDatagram(reinterpret_cast<char*>(&swlp_frame), sizeof(swlp_frame));

	// Verify SWLP frame
	if (swlp_frame.start_mark != SWLP_START_MARK_VALUE) {
		qDebug() << "[Wireless] Wrong SWLP_START_MARK_VALUE";
		return;
	}
	if (this->calculateCRC16(reinterpret_cast<const uint8_t*>(&swlp_frame), sizeof(swlp_frame)) != 0) {
		qDebug() << "[Wireless] Wrong CRC16";
		return;
	}

	// Process status payload
	memcpy(&m_status_payload, swlp_frame.payload, sizeof(m_status_payload));
	//emit status_payload_received(&m_status_payload);

	qDebug() << "[Wireless] Status payload received";
}

void Wireless::sendTimerEvent() {

	//emit command_frame_request(&m_command_payload);

	// Make command payload
	m_command_payload.command = 0xEE;

	// Make SWLP frame
	swlp_frame_t frame;
	memset(&frame, 0, sizeof(frame));

	frame.start_mark = SWLP_START_MARK_VALUE;
	frame.frame_number = 1;
	memcpy(frame.payload, &m_command_payload, sizeof(m_command_payload));
	frame.crc16 = 0;

	// Calculate CRC16
	uint16_t crc = this->calculateCRC16(reinterpret_cast<const uint8_t*>(&frame), sizeof(frame) - 2);
	frame.crc16 = crc;

	// Send SWLP frame
	QNetworkDatagram datagram;
	datagram.setDestination(QHostAddress(SERVER_IP_ADDRESS), SERVER_PORT);
	datagram.setData(QByteArray(reinterpret_cast<const char*>(&frame), sizeof(frame)));
	m_socket.writeDatagram(datagram);

	qDebug() << "[Wireless] Send command payload";
}

uint16_t Wireless::calculateCRC16(const uint8_t* frame, int size) {

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
