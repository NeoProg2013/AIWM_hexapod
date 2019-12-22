#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "core.h"


int main(int argc, char *argv[]) {

	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QGuiApplication app(argc, argv);
	Core g_core;

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("CppCore", &g_core);
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	if (engine.rootObjects().isEmpty()) {
		return -1;
	}

	return app.exec();
}
