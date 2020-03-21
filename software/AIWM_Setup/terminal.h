#ifndef TERMINAL_H
#define TERMINAL_H

#include <QObject>
#include <QUdpSocket>


class Terminal : public QObject
{
	Q_OBJECT
public:
	explicit Terminal(QObject* parent = nullptr);
	virtual ~Terminal();

	Q_INVOKABLE void runCommunication();
	Q_INVOKABLE void stopCommunication();
	Q_INVOKABLE void sendMessage(QVariant message);

signals:
	void messageReceived(QVariant message);

protected slots:
	void datagramReceivedEvent();

private:
	QUdpSocket m_socket;
};

#endif // WIRELESSMODBUS_H
