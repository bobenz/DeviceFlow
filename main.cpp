#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "state.h"
#include "deviceflow.h"
#include "trigger.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    // Register Status enum as a metatype
    qRegisterMetaType<StateBase::Status>("Status");
    qRegisterMetaType<StateBase::Status>("StateBase::Status");

    // Register C++ types with QML
    qmlRegisterType<StateBase>("DeviceFlow", 1, 0, "StateBase");
    qmlRegisterType<DeviceFlow>("DeviceFlow", 1, 0, "DeviceFlow");
    qmlRegisterType<Trigger>("DeviceFlow", 1, 0, "Trigger");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
