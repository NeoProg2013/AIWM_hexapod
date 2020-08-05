#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include "swlp.h"
#include "core.h"
#include "streamframeprovider.h"


int main(int argc, char *argv[]) {

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    StreamFrameProvider* streamFrameProvider = new StreamFrameProvider;
    Core core(streamFrameProvider);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("CppCore", &core);
    engine.addImageProvider("streamFrameProvider", streamFrameProvider);
    engine.load(QUrl(QStringLiteral("qrc:/AndroidQML/main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
