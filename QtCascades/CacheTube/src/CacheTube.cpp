#include <QtNetwork/QNetworkAccessManager>
#include <QtDeclarative/QDeclarativeEngine>

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>

#include "appsettings.h"
#include "invocationhelper.h"
#include "customtimer.h"
#include "ytvideomanager.h"
#include "ytarraydatamodel.h"

#include "CacheTube.hpp"

using namespace bb::cascades;

CacheTube::CacheTube(bb::cascades::Application *app) : QObject(app)
{
    QNetworkAccessManager *network_access_manager = new QNetworkAccessManager(this);
    AppSettings           *app_settings           = new AppSettings(this);

    qmlRegisterType<CustomTimer>("CustomTimer", 1, 0, "CustomTimer");

    qmlRegisterType<YTDownloadState>("YTVideoManagement", 1, 0, "YTDownloadState");
    qmlRegisterType<YTArrayDataModel>("YTVideoManagement", 1, 0, "YTArrayDataModel");

    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);

    qml->setContextProperty("AppSettings", app_settings);
    qml->setContextProperty("InvocationHelper", new InvocationHelper(this));
    qml->setContextProperty("YTVideoManager", new YTVideoManager(network_access_manager, app_settings->preferredVideoFormat(), this));

    AbstractPane *root = qml->createRootObject<AbstractPane>();

    app->setScene(root);
}
