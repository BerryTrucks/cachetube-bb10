#include <QtNetwork/QNetworkAccessManager>
#include <QtDeclarative/QDeclarativeEngine>

#include <bb/cascades/pickers/FilePicker>
#include <bb/cascades/pickers/FilePickerMode>
#include <bb/cascades/pickers/FilePickerSortFlag>
#include <bb/cascades/pickers/FilePickerSortOrder>
#include <bb/cascades/pickers/FilePickerViewMode>
#include <bb/cascades/pickers/FileType>

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>

#include "appsettings.h"
#include "playerlauncher.h"
#include "ytvideomanager.h"
#include "ytarraydatamodel.h"

#include "CacheTube.hpp"

using namespace bb::cascades;

CacheTube::CacheTube(bb::cascades::Application *app) : QObject(app)
{
    QNetworkAccessManager *network_access_manager = new QNetworkAccessManager(this);

    qmlRegisterType<PlayerLauncher>("PlayerLauncher", 1, 0, "PlayerLauncher");

    qmlRegisterType<YTDownloadState>("YTVideoManagement", 1, 0, "YTDownloadState");
    qmlRegisterType<YTArrayDataModel>("YTVideoManagement", 1, 0, "YTArrayDataModel");

    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);

    qml->setContextProperty("AppSettings", new AppSettings(this));
    qml->setContextProperty("YTVideoManager", new YTVideoManager(network_access_manager, this));

    AbstractPane *root = qml->createRootObject<AbstractPane>();

    app->setScene(root);
}
