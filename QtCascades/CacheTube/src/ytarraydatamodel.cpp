#include <algorithm>

#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QVariantMap>

#include "ytarraydatamodel.h"

static bool compareByTitle(QVariant &var1, QVariant &var2)
{
    QVariantMap map1 = var1.toMap();
    QVariantMap map2 = var2.toMap();

    if (map1.contains("title") && map2.contains("title")) {
        return (map1["title"].toString() < map2["title"].toString());
    } else {
        return false;
    }
}

static bool compareBySize(QVariant &var1, QVariant &var2)
{
    QVariantMap map1 = var1.toMap();
    QVariantMap map2 = var2.toMap();

    if (map1.contains("size") && map2.contains("size")) {
        return (map1["size"].toLongLong() > map2["size"].toLongLong());
    } else {
        return false;
    }
}

YTArrayDataModel::YTArrayDataModel(QObject *parent) : bb::cascades::ArrayDataModel(parent)
{
    UnwatchedFirst = true;
    SortOrder      = SortByTitle;
    VideoManager   = NULL;
}

YTArrayDataModel::~YTArrayDataModel()
{
}

int YTArrayDataModel::taskCount() const
{
    if (VideoManager != NULL) {
        return VideoManager->getTaskCount();
    } else {
        return 0;
    }
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

        if (VideoManager != NULL) {
            QObject::connect(VideoManager, SIGNAL(taskAdded(const YTDownloadTask&)),   this, SLOT(taskAdded(const YTDownloadTask&)));
            QObject::connect(VideoManager, SIGNAL(taskDeleted(const YTDownloadTask&)), this, SLOT(taskDeleted(const YTDownloadTask&)));
            QObject::connect(VideoManager, SIGNAL(taskChanged(const YTDownloadTask&)), this, SLOT(taskChanged(const YTDownloadTask&)));
        }

        FullUpdate();

        emit videoManagerChanged();
        emit taskCountChanged();
    }
}

void YTArrayDataModel::setUnwatchedFirst(const bool &first)
{
    UnwatchedFirst = first;

    FullUpdate();
}

void YTArrayDataModel::setSortOrder(const int &order)
{
    SortOrder = order;

    FullUpdate();
}

void YTArrayDataModel::setFilter(const QString &filter)
{
    Filter = filter;

    FullUpdate();
}

void YTArrayDataModel::taskAdded(const YTDownloadTask &task)
{
    Q_UNUSED(task);

    FullUpdate();

    emit taskCountChanged();
}

void YTArrayDataModel::taskDeleted(const YTDownloadTask &task)
{
    for (int i = 0; i < size(); i++) {
        QVariantMap map = value(i).value<QVariantMap>();

        if (map.contains("videoId") && map["videoId"].toString() == task.VideoId) {
            removeAt(i);

            break;
        }
    }

    emit taskCountChanged();
}

void YTArrayDataModel::taskChanged(const YTDownloadTask &task)
{
    for (int i = 0; i < size(); i++) {
        QVariantMap map = value(i).value<QVariantMap>();

        if (map.contains("videoId") && map["videoId"].toString() == task.VideoId) {
            if (map.contains("title") && map["title"].toString()  == task.Title &&
                map.contains("size")  && map["size"].toLongLong() == task.Size) {
                map["state"]    = task.State;
                map["done"]     = task.Done;
                map["errorMsg"] = task.ErrorMsg;

                replace(i, map);
            } else {
                FullUpdate();
            }

            break;
        }
    }
}

void YTArrayDataModel::FullUpdate()
{
    clear();

    if (VideoManager != NULL) {
        QList<YTDownloadTask> task_list = VideoManager->getTaskList();
        QVariantList          vmap_list;

        for (int i = 0; i < task_list.count(); i++) {
            if (Filter == "" || task_list.at(i).Title.contains(Filter, Qt::CaseInsensitive)) {
                QVariantMap map;

                map["state"]    = task_list.at(i).State;
                map["size"]     = task_list.at(i).Size;
                map["done"]     = task_list.at(i).Done;
                map["videoId"]  = task_list.at(i).VideoId;
                map["title"]    = task_list.at(i).Title;
                map["errorMsg"] = task_list.at(i).ErrorMsg;

                vmap_list.append(map);
            }
        }

        if (SortOrder == SortByTitle) {
            std::sort(vmap_list.begin(), vmap_list.end(), compareByTitle);
        } else if (SortOrder == SortBySize) {
            std::sort(vmap_list.begin(), vmap_list.end(), compareBySize);
        } else {
            std::sort(vmap_list.begin(), vmap_list.end(), compareByTitle);
        }

        append(vmap_list);
    }
}
