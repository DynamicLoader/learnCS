
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "QUtils.hpp"
#include "QmlExport.hpp"
#include "QImageMemory.hpp"
#include "ZIF.hpp"
#define WITH_IMPL
#include "WrapperTNAM.hpp"
#undef WITH_IMPL

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    qmlRegisterType<WrapperTNAM>("WrapperTNAM", 1, 0, "TNAM");
    qmlRegisterType<QImageMemory>("QImageMemory", 1, 0, "QImageMemory");
    qRegisterMetaType<QImageMemory *>("QImageMemory*");
    qmlRegisterSingletonType<QUtils>("QUtils",1,0,"QUtils",QUtils::qmlCreateInstance);
    qmlRegisterType<TNAM_Stat_t>("TNAM_Stat",1,0,"TNAM_Stat");
    qRegisterMetaType<cv::Mat>("cv::Mat");

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
