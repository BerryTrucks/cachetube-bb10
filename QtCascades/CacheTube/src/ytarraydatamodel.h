#ifndef YTARRAYDATAMODEL_H
#define YTARRAYDATAMODEL_H

#include <QtCore/QObject>

#include <bb/cascades/ArrayDataModel>

#include "ytvideomanager.h"

class YTArrayDataModel : public bb::cascades::ArrayDataModel {
    Q_OBJECT

    Q_PROPERTY(int      itemsCount   READ itemsCount                         NOTIFY itemsCountChanged)
    Q_PROPERTY(QObject* videoManager READ videoManager WRITE setVideoManager NOTIFY videoManagerChanged)

public:
    explicit YTArrayDataModel(QObject *parent = 0);
    virtual ~YTArrayDataModel();

    int itemsCount() const;

    QObject *videoManager() const;
    void     setVideoManager(QObject *video_manager);

public slots:
    void taskAdded(const YTDownloadTask &task);
    void taskDeleted(const YTDownloadTask &task);
    void taskChanged(const YTDownloadTask &task);

signals:
    void itemsCountChanged();
    void videoManagerChanged();

private:
    YTVideoManager *VideoManager;
};

#endif /* YTARRAYDATAMODEL_H */
