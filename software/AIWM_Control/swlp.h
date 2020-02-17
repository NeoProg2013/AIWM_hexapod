#ifndef WIRELESS_H
#define WIRELESS_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QEventLoop>
#include "swlp_protocol.h"


class Swlp : public QObject
{
	Q_OBJECT
public:
	explicit Swlp(QObject* parent = nullptr);
	virtual ~Swlp();

public slots:
	void runCommunication();

signals:
	void requestCommandPayload(swlp_command_payload_t* payload);
	void statusPayloadReceived(const swlp_status_payload_t* payload);


protected slots:
	void datagramReceivedEvent();
	void sendCommandPayloadEvent();

protected:
	uint16_t calculateCRC16(const uint8_t *frameByteArray, int size);

private:
	bool m_isRunning;
	QEventLoop* m_eventLoop;
	QUdpSocket* m_socket;
	QTimer* m_sendTimer;
	swlp_command_payload_t m_commandPayload;
	swlp_status_payload_t m_statusPayload;
};

#endif // WIRELESSMODBUS_H
