#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>
#include <QThread>
#include "swlp.h"
#include "core.h"
#include "terminal.h"


int main(int argc, char *argv[]) {

	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);

	Terminal terminal;
	Core core;

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("CppCore", &core);
	engine.rootContext()->setContextProperty("CppTerminal", &terminal);
	engine.load(QUrl(QStringLiteral("qrc:/AndroidQML/main.qml")));
	if (engine.rootObjects().isEmpty()) {
		return -1;
	}

	return app.exec();
}
