#ifndef WIRELESS_H
#define WIRELESS_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QThread>
#include "swlp_protocol.h"


class Wireless : public QThread
{
public:
	explicit Wireless(QObject* parent = nullptr);
	void run() override;

signals:
	void command_payload_request(swlp_command_payload_t* payload);
	void status_payload_received(const swlp_status_payload_t* payload);

protected slots:
	void datagramReceivedEvent();
	void sendTimerEvent();

protected:
	uint16_t calculateCRC16(const uint8_t *frameByteArray, int size);

private:
	QUdpSocket m_socket;
	QTimer m_sendTimer;
	swlp_command_payload_t m_command_payload;
	swlp_status_payload_t m_status_payload;
};

#endif // WIRELESSMODBUS_H
