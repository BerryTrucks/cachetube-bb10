#ifndef YTVIDEOMANAGER_H
#define YTVIDEOMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QIODevice>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtSql/QSqlDatabase>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

class YTDownloadState : public QObject
{
    Q_OBJECT

    Q_ENUMS(DownloadState)

public:
    enum DownloadState {
        StateActive,
        StateCompleted,
        StateError,
        StateQueued,
        StatePaused
    };
};

struct YTDownloadTask
{
    bool    Watched;
    int     State, Fmt;
    qint64  Size, Done, StartTime;
    QString VideoId, Title, ErrorMsg, JSPlayerURL, VideoURL, Signature, VisitorInfo1LiveCookie;
};

class YTVideoManager : public QObject
{
    Q_OBJECT

public:
    explicit YTVideoManager(QNetworkAccessManager *network_access_manager, int preferred_format, QObject *parent = 0);
    virtual ~YTVideoManager();

    Q_INVOKABLE void setPreferredVideoFormat(const int &format);

    Q_INVOKABLE QString getVideoId(const QString &url);

    Q_INVOKABLE bool addTask(const QString &video_id);
    Q_INVOKABLE void delTask(const QString &video_id);

    Q_INVOKABLE bool restTask(const QString &video_id);
    Q_INVOKABLE void wipeTask(const QString &video_id);

    Q_INVOKABLE void pauseTask(const QString &video_id);
    Q_INVOKABLE void resumeTask(const QString &video_id);

    Q_INVOKABLE void setTaskWatched(const QString &video_id);

    Q_INVOKABLE QString getTaskWebURL(const QString &video_id);
    Q_INVOKABLE QString getTaskVideoURI(const QString &video_id);

    Q_INVOKABLE int                   getTaskCount();
    Q_INVOKABLE QList<YTDownloadTask> getTaskList();

private slots:
    void runQueue();

    void networkRequestFinished(QNetworkReply *reply);

    void downloadProgress(qint64 received, qint64 total);
    void downloadDataReady();

signals:
    void taskAdded(const YTDownloadTask &task);
    void taskDeleted(const YTDownloadTask &task);
    void taskChanged(const YTDownloadTask &task);

private:
    bool ReopenCurrentFile(bool *file_valid);

    void UpdateTask(const YTDownloadTask &task);

    bool ParseMetadata(const QByteArray &raw_data, QString *video_title, QHash<int, QString> *fmt_url_map, QHash<int, QString> *fmt_sig_map);
    bool ParseVideoPage(const QByteArray &raw_data, QString *sts, QString *js_player_url);
    bool DecodeSignature(const QByteArray &raw_js_code, const QString &encoded_signature, QString *decoded_signature);

    QString FormatNetworkError(QNetworkReply::NetworkError error);

    static const int QUEUE_RUN_AFTER = 1000;

    int                   PreferredVideoFormat;
    QDir                  DestinationDir;
    QList<YTDownloadTask> ActiveTasks, DeletedTasks;
    YTDownloadTask        CurrentTask;
    QFile                 CurrentFile;
    QSqlDatabase          TaskDatabase;
    QNetworkAccessManager *NetworkAccessManager;
    QNetworkReply         *VideoPageReply, *MetadataReply, *JSPlayerReply, *DownloadReply;
};

#endif // YTVIDEOMANAGER_H
