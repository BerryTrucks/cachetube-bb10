#include <QtCore/QList>
#include <QtCore/QVariantMap>

#include "ytarraydatamodel.h"

YTArrayDataModel::YTArrayDataModel(QObject *parent) : bb::cascades::ArrayDataModel(parent)
{
    VideoManager = NULL;
}

YTArrayDataModel::~YTArrayDataModel()
{
}

int YTArrayDataModel::itemsCount() const
{
    return size();
}

QObject *YTArrayDataModel::videoManager() const
{
    return VideoManager;
}

void YTArrayDataModel::setVideoManager(QObject *video_manager)
{
    YTVideoManager *manager = dynamic_cast<YTVideoManager*>(video_manager);

    if (VideoManager != manager) {
        if (VideoManager != NULL) {
            QObject::disconnect(VideoManager, SIGNAL(taskAdded(const YTDownloadTask&)),   this, SLOT(taskAdded(const YTDownloadTask&)));
            QObject::disconnect(VideoManager, SIGNAL(taskDeleted(const YTDownloadTask&)), this, SLOT(taskDeleted(const YTDownloadTask&)));
            QObject::disconnect(VideoManager, SIGNAL(taskChanged(const YTDownloadTask&)), this, SLOT(taskChanged(const YTDownloadTask&)));
        }

        VideoManager = manager;

        emit videoManagerChanged();

        int items_count = size();

        clear();

        if (VideoManager != NULL) {
            QList<YTDownloadTask> task_list = VideoManager->getTaskList();

            for (int i = 0; i < task_list.count(); i++) {
                QVariantMap map;

                map["state"]    = task_list.at(i).State;
                map["size"]     = task_list.at(i).Size;
                map["done"]     = task_list.at(i).Done;
                map["videoId"]  = task_list.at(i).VideoId;
                map["mimeType"] = task_list.at(i).MimeType;
                map["title"]    = task_list.at(i).Title;
                map["errorMsg"] = task_list.at(i).ErrorMsg;

                append(map);
            }

            QObject::connect(VideoManager, SIGNAL(taskAdded(const YTDownloadTask&)),   this, SLOT(taskAdded(const YTDownloadTask&)));
            QObject::connect(VideoManager, SIGNAL(taskDeleted(const YTDownloadTask&)), this, SLOT(taskDeleted(const YTDownloadTask&)));
            QObject::connect(VideoManager, SIGNAL(taskChanged(const YTDownloadTask&)), this, SLOT(taskChanged(const YTDownloadTask&)));
        }

        if (size() != items_count) {
            emit itemsCountChanged();
        }
    }
}

void YTArrayDataModel::taskAdded(const YTDownloadTask &task)
{
    QVariantMap map;

    map["state"]    = task.State;
    map["size"]     = task.Size;
    map["done"]     = task.Done;
    map["videoId"]  = task.VideoId;
    map["mimeType"] = task.MimeType;
    map["title"]    = task.Title;
    map["errorMsg"] = task.ErrorMsg;

    append(map);

    emit itemsCountChanged();
}

void YTArrayDataModel::taskDeleted(const YTDownloadTask &task)
{
    for (int i = 0; i < size(); i++) {
        QVariantMap map = value(i).value<QVariantMap>();

        if (map.contains("videoId") && map["videoId"].toString() == task.VideoId) {
            removeAt(i);

            emit itemsCountChanged();

            break;
        }
    }
}

void YTArrayDataModel::taskChanged(const YTDownloadTask &task)
{
    for (int i = 0; i < size(); i++) {
        QVariantMap map = value(i).value<QVariantMap>();

        if (map.contains("videoId") && map["videoId"].toString() == task.VideoId) {
            map["state"]    = task.State;
            map["size"]     = task.Size;
            map["done"]     = task.Done;
            map["mimeType"] = task.MimeType;
            map["title"]    = task.Title;
            map["errorMsg"] = task.ErrorMsg;

            replace(i, map);

            break;
        }
    }
}
