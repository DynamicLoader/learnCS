/* https://doc.qt.io/qt-6/qtquick-visualcanvas-scenegraph-renderer.html */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>

#include "QImageMemory.hpp"
#include "QUtils.hpp"

#define WITH_IMPL
#include "WrapperTNAM.hpp"
#include "WrapperRNAM.hpp"
#undef WITH_IMPL

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    qmlRegisterType<WrapperTNAM>("WrapperTNAM", 1, 0, "TNAM");
    qmlRegisterType<WrapperRNAM>("WrapperRNAM", 1, 0, "RNAM");
    qmlRegisterType<QImageMemory>("QImageMemory", 1, 0, "QImageMemory");
    qmlRegisterSingletonType<QUtils>("QUtils", 1, 0, "QUtils", QUtils::qmlCreateInstance);
    qmlRegisterType<TNAM_Stat_t>("TNAM_Stat", 1, 0, "TNAM_Stat");
    qmlRegisterType<RNAM_Stat_t>("RNAM_Stat", 1, 0, "RNAM_Stat");

    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<QImageMemory *>("QImageMemory*");

    app.setWindowIcon(QIcon(":/ui/img/icon.png"));

    const QUrl url(u"qrc:/ui/main.qml"_qs);
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
