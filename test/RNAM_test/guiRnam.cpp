// Qlibrary
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>

#include <QQmlContext>
#include <qapplication.h>
#include <qqml.h>
#include <qqmlapplicationengine.h>
#include <qurl.h>
// RNAM support
#include "QmlExport.hpp"
#include "QImageMemory.hpp"
#include "WrapperRNAM.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    const QUrl url(u"qrc:///main.qml"_qs);

    qmlRegisterType<WrapperRNAM>("WrapperRNAM", 1, 0, "WrapperRNAM");
    qmlRegisterType<QImageMemory>("QImageMemory", 1, 0, "QImageMemory");
    qmlRegisterType<RNAM_Stat_t>("RNAM_Stat", 1, 0, "RNAM_Stat");
    qRegisterMetaType<QImageMemory *>("QImageMemory*");

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}