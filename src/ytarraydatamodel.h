#ifndef YTARRAYDATAMODEL_H
#define YTARRAYDATAMODEL_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include <bb/cascades/ArrayDataModel>

#include "ytvideomanager.h"

class YTArrayDataModel : public bb::cascades::ArrayDataModel {
    Q_OBJECT

    Q_PROPERTY(int      taskCount    READ taskCount                          NOTIFY taskCountChanged)
    Q_PROPERTY(QObject* videoManager READ videoManager WRITE setVideoManager NOTIFY videoManagerChanged)

    Q_ENUMS(SortOrder)

public:
    explicit YTArrayDataModel(QObject *parent = 0);
    virtual ~YTArrayDataModel();

    int taskCount() const;

    QObject *videoManager() const;
    void     setVideoManager(QObject *video_manager);

    Q_INVOKABLE void setUnwatchedFirst(const bool &first);
    Q_INVOKABLE void setSortOrder(const int &order);
    Q_INVOKABLE void setFilter(const QString &filter);

    enum SortOrder {
        SortByStartTime,
        SortByTitle,
        SortBySize
    };

public slots:
    void taskAdded(const YTDownloadTask &task);
    void taskDeleted(const YTDownloadTask &task);
    void taskChanged(const YTDownloadTask &task);

signals:
    void taskCountChanged();
    void videoManagerChanged();

private:
    void FullUpdate();

    bool           UnwatchedFirst;
    int            SortOrder;
    QString        Filter;
    YTVideoManager *VideoManager;
};

#endif /* YTARRAYDATAMODEL_H */
