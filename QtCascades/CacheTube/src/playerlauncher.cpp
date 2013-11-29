#include <QtCore/QVariantMap>

#include <bb/PpsObject>
#include <bb/system/InvokeManager>
#include <bb/system/InvokeRequest>
#include <bb/system/InvokeTargetReply>

#include "playerlauncher.h"

PlayerLauncher::PlayerLauncher(QObject *parent) : bb::cascades::CustomControl()
{
    Q_UNUSED(parent);
}

PlayerLauncher::~PlayerLauncher()
{
}

bool PlayerLauncher::launchPlayer(const QString &video_uri, const QString &video_mime_type, const QString &video_title)
{
    bb::system::InvokeManager      invoke_manager;
    bb::system::InvokeRequest      invoke_request;
    bb::system::InvokeTargetReply *invoke_reply;
    QVariantMap                    map;

    map.insert("contentTitle", video_title);

    invoke_request.setAction("bb.action.OPEN");
    invoke_request.setTarget("sys.mediaplayer.previewer");
    invoke_request.setMimeType(video_mime_type);
    invoke_request.setData(bb::PpsObject::encode(map));
    invoke_request.setFileTransferMode(bb::system::FileTransferMode::Preserve);
    invoke_request.setUri(video_uri);

    invoke_reply = invoke_manager.invoke(invoke_request);

    if (invoke_reply != NULL) {
        QObject::connect(invoke_reply, SIGNAL(finished()), this, SLOT(invokeFinished()));

        return true;
    } else {
        return false;
    }
}

void PlayerLauncher::invokeFinished()
{
    bb::system::InvokeTargetReply *invoke_reply = qobject_cast<bb::system::InvokeTargetReply*>(sender());

    if (invoke_reply != NULL) {
        invoke_reply->deleteLater();
    }
}
