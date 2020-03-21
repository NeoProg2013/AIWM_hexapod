#include <QHostAddress>
#include <QNetworkDatagram>
#include "terminal.h"
#define SERVER_IP_ADDRESS					("111.111.111.111")
#define SERVER_PORT							(3333)


Terminal::Terminal(QObject* parent) : QObject(parent) {

}

Terminal::~Terminal() {
	stopCommunication();
}

void Terminal::runCommunication() {

	// Setup UDP socket
	m_socket.open(QUdpSocket::ReadWrite);
	if (m_socket.bind(SERVER_PORT) == false) {
		return;
	}
	connect(&m_socket, &QUdpSocket::readyRead, this, &Terminal::datagramReceivedEvent);

	// Switch to CLI interface
	sendMessage("cli");
}

void Terminal::stopCommunication() {

	// Switch to SWLP interface
	sendMessage("exit");

	m_socket.disconnectFromHost();
	disconnect(&m_socket, &QUdpSocket::readyRead, this, &Terminal::datagramReceivedEvent);
}

void Terminal::sendMessage(QVariant message) {

	// Send SWLP frame
	QNetworkDatagram datagram;
	datagram.setDestination(QHostAddress(SERVER_IP_ADDRESS), SERVER_PORT);
	datagram.setData(message.toString().toLower().toUtf8());
	m_socket.writeDatagram(datagram);

	m_socket.waitForBytesWritten(1000);
}

void Terminal::datagramReceivedEvent() {

	char buffer[1024] = {0};
	m_socket.readDatagram(buffer, sizeof(buffer));

	QString message(buffer);
	emit messageReceived(message);
}
