#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>
#include <QDebug>
#include <QThread>
#include "swlp.h"
#include "core.h"


int main(int argc, char *argv[]) {

	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

	Core core;

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("CppCore", &core);
	engine.load(QUrl(QStringLiteral("qrc:/AndroidQML/main.qml")));
	if (engine.rootObjects().isEmpty()) {
		return -1;
	}

	return app.exec();
}
