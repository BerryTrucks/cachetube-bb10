#include "invocationhelper.h"

InvocationHelper::InvocationHelper(QObject *parent) : QObject(parent)
{
    InvocationManager = new bb::system::InvokeManager();

    QObject::connect(InvocationManager, SIGNAL(invoked(const bb::system::InvokeRequest &)), this, SLOT(invoked(const bb::system::InvokeRequest &)));
}

InvocationHelper::~InvocationHelper()
{
    delete InvocationManager;
}

QUrl InvocationHelper::videoUrl() const
{
    return VideoURL;
}

void InvocationHelper::setVideoUrl(const QUrl &video_url)
{
    if (VideoURL != video_url) {
        VideoURL = video_url;

        emit videoUrlChanged();
    }
}

void InvocationHelper::invoked(const bb::system::InvokeRequest &request)
{
    VideoURL = request.uri();

    emit videoUrlChanged();
}
