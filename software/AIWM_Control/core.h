#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QTimer>
#include <QFuture>
#include "wireless.h"

class Core : public QObject {

	Q_OBJECT

public:
	explicit Core(QObject *parent = nullptr);
	virtual ~Core();
	Q_INVOKABLE bool connectToServer();
	Q_INVOKABLE void disconnectFromServer();

	Q_INVOKABLE void sendGetUpCommand();
	Q_INVOKABLE void sendGetDownCommand();
	Q_INVOKABLE void sendRunCommand();
	Q_INVOKABLE void sendDirectMoveCommand();
	Q_INVOKABLE void sendReverseMoveCommand();
	Q_INVOKABLE void sendRotateLeftCommand();
	Q_INVOKABLE void sendRotateRightCommand();
	Q_INVOKABLE void sendDirectMoveSlowCommand();
	Q_INVOKABLE void sendReverseMoveSlowCommand();
	Q_INVOKABLE void sendShiftLeftCommand();
	Q_INVOKABLE void sendShiftRightCommand();

	Q_INVOKABLE void sendAttackLeftCommand();
	Q_INVOKABLE void sendAttackRightCommand();
	Q_INVOKABLE void sendDanceCommand();
	Q_INVOKABLE void sendRotateXCommand();
	Q_INVOKABLE void sendRotateZCommand();
	Q_INVOKABLE void sendStopMoveCommand();

	Q_INVOKABLE bool sendEnableFrontDistanceSensorCommand();
	Q_INVOKABLE bool sendDisableFrontDistanceSensorCommand();

signals:
	void systemStatusUpdatedSignal(QVariant newSystemStatus);
	void systemVoltageUpdatedSignal(int wireless, int periph, int battery);
	void connectToServerSignal();
	void disconnectFromServerSignal();
	void writeDataToRamSignal(int address, QByteArray data);
	void readDataFromRamSignal(int address, QByteArray* data, int bytesCount);

public slots:
	void statusUpdateTimer();

protected:
	bool writeToSCR(int cmd, int retryCount);
	void waitOperationCompleted();

protected:
	QThread m_thread;
	//WirelessModbus* m_wirelessModbus;
	QTimer m_statusUpdateTimer;
	QFuture<bool> m_concurrentFuture;
};

#endif // CORE_H
