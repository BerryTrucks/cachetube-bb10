#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/QTimer>
#include <QtSql/QSqlQuery>
#include <QtNetwork/QNetworkRequest>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

#include <bb/data/JsonDataAccess>

#include "ytvideomanager.h"

YTVideoManager::YTVideoManager(QNetworkAccessManager *network_access_manager, int preferred_format, QObject *parent) : QObject(parent)
{
    DestinationDir = QDir(QDir::homePath() + QDir::separator() + "ytvideo");

    if (!DestinationDir.exists()) {
        QDir::home().mkpath("ytvideo");
    }

    TaskDatabase = QSqlDatabase::addDatabase("QSQLITE");

    TaskDatabase.setDatabaseName(QDir::homePath() + QDir::separator() + "ytvideo.db");

    if (TaskDatabase.open()) {
        QSqlQuery query(TaskDatabase);

        if (query.exec("CREATE TABLE IF NOT EXISTS TASKS(VIDEO_ID TEXT, STATE INTEGER, FMT INTEGER, SIZE INTEGER, DONE INTEGER, TITLE TEXT)") &&
            query.exec("CREATE INDEX IF NOT EXISTS TASKS_VIDEO_ID ON TASKS(VIDEO_ID)")) {
            if (query.exec("SELECT VIDEO_ID, STATE, FMT, SIZE, DONE, TITLE FROM TASKS")) {
                while(query.next()) {
                    YTDownloadTask task;

                    task.State                  = query.value(1).toInt();
                    task.Fmt                    = query.value(2).toInt();
                    task.Size                   = query.value(3).toLongLong();
                    task.Done                   = query.value(4).toLongLong();
                    task.VideoId                = query.value(0).toString();
                    task.Title                  = query.value(5).toString();
                    task.ErrorMsg               = "";
                    task.JSPlayerURL            = "";
                    task.VideoURL               = "";
                    task.Signature              = "";
                    task.VisitorInfo1LiveCookie = "";

                    if (task.State != YTDownloadState::StateCompleted && task.State != YTDownloadState::StatePaused) {
                        task.State = YTDownloadState::StateQueued;
                    }

                    ActiveTasks.append(task);
                }
            }
        }

        TaskDatabase.close();
    }

    PreferredVideoFormat = preferred_format;
    NetworkAccessManager = network_access_manager;
    VideoPageReply       = NULL;
    MetadataReply        = NULL;
    JSPlayerReply        = NULL;
    DownloadReply        = NULL;

    QObject::connect(NetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkRequestFinished(QNetworkReply*)));

    QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
}

YTVideoManager::~YTVideoManager()
{
    for (int i = 0; i < DeletedTasks.size(); i++) {
        QFile::remove(DestinationDir.path() + QDir::separator() + DeletedTasks.at(i).VideoId);
    }

    if (VideoPageReply != NULL) {
        delete VideoPageReply;
    }
    if (MetadataReply != NULL) {
        delete MetadataReply;
    }
    if (JSPlayerReply != NULL) {
        delete JSPlayerReply;
    }
    if (DownloadReply != NULL) {
        delete DownloadReply;
    }
}

void YTVideoManager::setPreferredVideoFormat(const int &format)
{
    PreferredVideoFormat = format;
}

QString YTVideoManager::getVideoId(const QString &url)
{
    QRegExp validator   = QRegExp("/watch\\?");
    QRegExp extractor_1 = QRegExp("&v=([^&]+)");
    QRegExp extractor_2 = QRegExp("\\?v=([^&]+)");

    if (validator.indexIn(url) != -1) {
        if (extractor_1.indexIn(url) != -1) {
            return extractor_1.cap(1);
        } else if (extractor_2.indexIn(url) != -1) {
            return extractor_2.cap(1);
        } else {
            return "";
        }
    } else {
        return "";
    }
}

bool YTVideoManager::addTask(const QString &video_id)
{
    if (video_id.isEmpty()) {
        return false;
    } else {
        for (int i = 0; i < ActiveTasks.size(); i++) {
            if (ActiveTasks.at(i).VideoId == video_id) {
                return false;
            }
        }
        for (int i = 0; i < DeletedTasks.size(); i++) {
            if (DeletedTasks.at(i).VideoId == video_id) {
                QFile::remove(DestinationDir.path() + QDir::separator() + DeletedTasks.at(i).VideoId);

                DeletedTasks.removeAt(i);

                break;
            }
        }

        YTDownloadTask task;

        task.State                  = YTDownloadState::StateQueued;
        task.Fmt                    = 0;
        task.Size                   = 0;
        task.Done                   = 0;
        task.VideoId                = video_id;
        task.Title                  = getTaskWebURL(video_id);
        task.ErrorMsg               = "";
        task.JSPlayerURL            = "";
        task.VideoURL               = "";
        task.Signature              = "";
        task.VisitorInfo1LiveCookie = "";

        ActiveTasks.append(task);

        emit taskAdded(task);

        if (TaskDatabase.open()) {
            QSqlQuery query(TaskDatabase);

            query.prepare("INSERT INTO TASKS(VIDEO_ID, STATE, FMT, SIZE, DONE, TITLE) VALUES (:VIDEO_ID, :STATE, :FMT, :SIZE, :DONE, :TITLE)");

            query.bindValue(":VIDEO_ID", task.VideoId);
            query.bindValue(":STATE",    task.State);
            query.bindValue(":FMT",      task.Fmt);
            query.bindValue(":SIZE",     task.Size);
            query.bindValue(":DONE",     task.Done);
            query.bindValue(":TITLE",    task.Title);

            query.exec();

            TaskDatabase.close();
        }

        QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));

        return true;
    }
}

void YTVideoManager::delTask(const QString &video_id)
{
    for (int i = 0; i < ActiveTasks.size(); i++) {
        if (ActiveTasks.at(i).VideoId == video_id) {
            for (int j = 0; j < DeletedTasks.size(); j++) {
                if (DeletedTasks.at(j).VideoId == video_id) {
                    QFile::remove(DestinationDir.path() + QDir::separator() + DeletedTasks.at(j).VideoId);

                    DeletedTasks.removeAt(j);

                    break;
                }
            }

            if (CurrentTask.VideoId == video_id) {
                if (VideoPageReply != NULL) {
                    VideoPageReply->abort();
                }
                if (MetadataReply != NULL) {
                    MetadataReply->abort();
                }
                if (JSPlayerReply != NULL) {
                    JSPlayerReply->abort();
                }
                if (DownloadReply != NULL) {
                    DownloadReply->abort();
                }
            }

            YTDownloadTask task = ActiveTasks.at(i);

            DeletedTasks.append(task);

            ActiveTasks.removeAt(i);

            emit taskDeleted(task);

            if (TaskDatabase.open()) {
                QSqlQuery query(TaskDatabase);

                query.prepare("DELETE FROM TASKS WHERE VIDEO_ID=:VIDEO_ID");

                query.bindValue(":VIDEO_ID", video_id);

                query.exec();

                TaskDatabase.close();
            }

            break;
        }
    }

    QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
}

bool YTVideoManager::restTask(const QString &video_id)
{
    for (int i = 0; i < DeletedTasks.size(); i++) {
        if (DeletedTasks.at(i).VideoId == video_id) {
            for (int j = 0; j < ActiveTasks.size(); j++) {
                if (ActiveTasks.at(j).VideoId == video_id) {
                    return false;
                }
            }

            YTDownloadTask task = DeletedTasks.at(i);

            if (task.State != YTDownloadState::StateCompleted && task.State != YTDownloadState::StatePaused) {
                task.State = YTDownloadState::StateQueued;
            }

            ActiveTasks.append(task);

            DeletedTasks.removeAt(i);

            emit taskAdded(task);

            if (TaskDatabase.open()) {
                QSqlQuery query(TaskDatabase);

                query.prepare("INSERT INTO TASKS(VIDEO_ID, STATE, FMT, SIZE, DONE, TITLE) VALUES (:VIDEO_ID, :STATE, :FMT, :SIZE, :DONE, :TITLE)");

                query.bindValue(":VIDEO_ID", task.VideoId);
                query.bindValue(":STATE",    task.State);
                query.bindValue(":FMT",      task.Fmt);
                query.bindValue(":SIZE",     task.Size);
                query.bindValue(":DONE",     task.Done);
                query.bindValue(":TITLE",    task.Title);

                query.exec();

                TaskDatabase.close();
            }

            QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));

            return true;
        }
    }

    return false;
}

void YTVideoManager::wipeTask(const QString &video_id)
{
    for (int i = 0; i < DeletedTasks.size(); i++) {
        if (DeletedTasks.at(i).VideoId == video_id) {
            QFile::remove(DestinationDir.path() + QDir::separator() + DeletedTasks.at(i).VideoId);

            DeletedTasks.removeAt(i);

            break;
        }
    }
}

void YTVideoManager::pauseTask(const QString &video_id)
{
    for (int i = 0; i < ActiveTasks.size(); i++) {
        if (ActiveTasks.at(i).VideoId == video_id && ActiveTasks.at(i).State != YTDownloadState::StateCompleted &&
                                                     ActiveTasks.at(i).State != YTDownloadState::StatePaused) {
            if (CurrentTask.VideoId == video_id) {
                if (VideoPageReply != NULL) {
                    VideoPageReply->abort();
                }
                if (MetadataReply != NULL) {
                    MetadataReply->abort();
                }
                if (JSPlayerReply != NULL) {
                    JSPlayerReply->abort();
                }
                if (DownloadReply != NULL) {
                    DownloadReply->abort();
                }
            }

            YTDownloadTask task = ActiveTasks.at(i);

            task.State = YTDownloadState::StatePaused;

            ActiveTasks.replace(i, task);

            emit taskChanged(task);

            if (TaskDatabase.open()) {
                QSqlQuery query(TaskDatabase);

                query.prepare("UPDATE TASKS SET STATE=:STATE WHERE VIDEO_ID=:VIDEO_ID");

                query.bindValue(":STATE",    task.State);
                query.bindValue(":VIDEO_ID", task.VideoId);

                query.exec();

                TaskDatabase.close();
            }

            break;
        }
    }

    QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
}

void YTVideoManager::resumeTask(const QString &video_id)
{
    for (int i = 0; i < ActiveTasks.size(); i++) {
        if (ActiveTasks.at(i).VideoId == video_id && ActiveTasks.at(i).State == YTDownloadState::StatePaused) {
            YTDownloadTask task = ActiveTasks.at(i);

            task.State = YTDownloadState::StateQueued;

            ActiveTasks.replace(i, task);

            emit taskChanged(task);

            if (TaskDatabase.open()) {
                QSqlQuery query(TaskDatabase);

                query.prepare("UPDATE TASKS SET STATE=:STATE WHERE VIDEO_ID=:VIDEO_ID");

                query.bindValue(":STATE",    task.State);
                query.bindValue(":VIDEO_ID", task.VideoId);

                query.exec();

                TaskDatabase.close();
            }

            break;
        }
    }

    QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
}

QString YTVideoManager::getTaskWebURL(const QString &video_id)
{
    return QString("http://m.youtube.com/watch?v=%1").arg(video_id);
}

QString YTVideoManager::getTaskVideoURI(const QString &video_id)
{
    for (int i = 0; i < ActiveTasks.size(); i++) {
        if (ActiveTasks.at(i).VideoId == video_id) {
            return QUrl::fromLocalFile(DestinationDir.path() + QDir::separator() + ActiveTasks.at(i).VideoId).toString();
        }
    }

    return "";
}

int YTVideoManager::getTaskCount()
{
    return ActiveTasks.count();
}

QList<YTDownloadTask> YTVideoManager::getTaskList()
{
    return ActiveTasks;
}

void YTVideoManager::runQueue()
{
    if (VideoPageReply == NULL && MetadataReply == NULL && JSPlayerReply == NULL && DownloadReply == NULL) {
        for (int i = 0; i < ActiveTasks.size(); i++) {
            if (ActiveTasks.at(i).State != YTDownloadState::StateCompleted && ActiveTasks.at(i).State != YTDownloadState::StatePaused) {
                CurrentTask = ActiveTasks.at(i);

                QNetworkRequest request(QUrl::fromEncoded(QString("https://www.youtube.com/watch?v=%1&gl=US&hl=en&has_verified=1").arg(CurrentTask.VideoId).toAscii()));

                request.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);

                VideoPageReply = NetworkAccessManager->get(request);

                break;
            }
        }
    }
}

void YTVideoManager::networkRequestFinished(QNetworkReply *reply)
{
    if (reply == VideoPageReply) {
        if (reply->error() == QNetworkReply::NoError) {
            QString sts;
            QString js_player_url;

            if (reply->hasRawHeader("Set-Cookie")) {
                QString cookie                       = reply->rawHeader("Set-Cookie");
                QRegExp visitor_info1_live_extractor = QRegExp("VISITOR_INFO1_LIVE=([^;]+)");

                if (visitor_info1_live_extractor.indexIn(cookie) != -1) {
                    CurrentTask.VisitorInfo1LiveCookie = visitor_info1_live_extractor.cap(1);

                    UpdateTask(CurrentTask);
                }
            }

            if (ParseVideoPage(reply->readAll(), &sts, &js_player_url)) {
                CurrentTask.JSPlayerURL = js_player_url;

                UpdateTask(CurrentTask);

                QNetworkRequest request(QUrl::fromEncoded(QString("https://www.youtube.com/get_video_info?&video_id=%1&eurl=%2&sts=%3").arg(CurrentTask.VideoId, QString(QUrl::toPercentEncoding(QString("https://youtube.googleapis.com/v/%1").arg(CurrentTask.VideoId))), sts).toAscii()));

                if (!CurrentTask.VisitorInfo1LiveCookie.isEmpty()) {
                    request.setRawHeader("Cookie", QString("VISITOR_INFO1_LIVE=%1").arg(CurrentTask.VisitorInfo1LiveCookie).toAscii());
                }

                MetadataReply = NetworkAccessManager->get(request);
            } else {
                CurrentTask.State    = YTDownloadState::StateError;
                CurrentTask.ErrorMsg = tr("Cannot extract video information");

                UpdateTask(CurrentTask);

                QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
            }
        } else {
            if (reply->error() != QNetworkReply::OperationCanceledError) {
                CurrentTask.State    = YTDownloadState::StateError;
                CurrentTask.ErrorMsg = FormatNetworkError(reply->error());

                UpdateTask(CurrentTask);
            }

            QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
        }

        VideoPageReply = NULL;

        reply->deleteLater();
    } else if (reply == MetadataReply) {
        if (reply->error() == QNetworkReply::NoError) {
            QString             video_title;
            QHash<int, QString> fmt_url_map;
            QHash<int, QString> fmt_sig_map;

            if (reply->hasRawHeader("Set-Cookie")) {
                QString cookie                       = reply->rawHeader("Set-Cookie");
                QRegExp visitor_info1_live_extractor = QRegExp("VISITOR_INFO1_LIVE=([^;]+)");

                if (visitor_info1_live_extractor.indexIn(cookie) != -1) {
                    CurrentTask.VisitorInfo1LiveCookie = visitor_info1_live_extractor.cap(1);

                    UpdateTask(CurrentTask);
                }
            }

            if (ParseMetadata(reply->readAll(), &video_title, &fmt_url_map, &fmt_sig_map)) {
                CurrentTask.Title = video_title;

                if (CurrentTask.Fmt == 0) {
                    if (PreferredVideoFormat == 18) {
                        if (fmt_url_map.contains(18)) {
                            CurrentTask.Fmt = 18;
                        }
                    } else {
                        if (fmt_url_map.contains(22)) {
                            CurrentTask.Fmt = 22;
                        } else if (fmt_url_map.contains(18)) {
                            CurrentTask.Fmt = 18;
                        }
                    }
                }

                UpdateTask(CurrentTask);

                if (CurrentTask.Fmt != 0 && fmt_url_map.contains(CurrentTask.Fmt)) {
                    if (fmt_sig_map.contains(CurrentTask.Fmt)) {
                        if (!CurrentTask.JSPlayerURL.isEmpty()) {
                            CurrentTask.VideoURL  = fmt_url_map[CurrentTask.Fmt];
                            CurrentTask.Signature = fmt_sig_map[CurrentTask.Fmt];

                            UpdateTask(CurrentTask);

                            QNetworkRequest request(QUrl::fromEncoded(CurrentTask.JSPlayerURL.toAscii()));

                            if (!CurrentTask.VisitorInfo1LiveCookie.isEmpty()) {
                                request.setRawHeader("Cookie", QString("VISITOR_INFO1_LIVE=%1").arg(CurrentTask.VisitorInfo1LiveCookie).toAscii());
                            }

                            JSPlayerReply = NetworkAccessManager->get(request);
                        } else {
                            CurrentTask.State    = YTDownloadState::StateError;
                            CurrentTask.ErrorMsg = tr("Cannot download signature decoder");

                            UpdateTask(CurrentTask);

                            QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
                        }
                    } else {
                        bool file_valid;

                        if (ReopenCurrentFile(&file_valid)) {
                            QNetworkRequest request(QUrl::fromEncoded(fmt_url_map[CurrentTask.Fmt].toAscii()));

                            if (!CurrentTask.VisitorInfo1LiveCookie.isEmpty()) {
                                request.setRawHeader("Cookie", QString("VISITOR_INFO1_LIVE=%1").arg(CurrentTask.VisitorInfo1LiveCookie).toAscii());
                            }
                            if (file_valid) {
                                request.setRawHeader("Range", QString("bytes=%1-").arg(CurrentTask.Done).toAscii());
                            }

                            DownloadReply = NetworkAccessManager->get(request);

                            QObject::connect(DownloadReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
                            QObject::connect(DownloadReply, SIGNAL(readyRead()),                      this, SLOT(downloadDataReady()));
                        } else {
                            QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
                        }
                    }
                } else {
                    CurrentTask.State    = YTDownloadState::StateError;
                    CurrentTask.ErrorMsg = tr("No suitable video formats available");

                    UpdateTask(CurrentTask);

                    QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
                }
            } else {
                CurrentTask.State    = YTDownloadState::StateError;
                CurrentTask.ErrorMsg = tr("No suitable video formats available");

                UpdateTask(CurrentTask);

                QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
            }
        } else {
            if (reply->error() != QNetworkReply::OperationCanceledError) {
                CurrentTask.State    = YTDownloadState::StateError;
                CurrentTask.ErrorMsg = FormatNetworkError(reply->error());

                UpdateTask(CurrentTask);
            }

            QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
        }

        MetadataReply = NULL;

        reply->deleteLater();
    } else if (reply == JSPlayerReply) {
        if (reply->error() == QNetworkReply::NoError) {
            QString signature;

            if (reply->hasRawHeader("Set-Cookie")) {
                QString cookie                       = reply->rawHeader("Set-Cookie");
                QRegExp visitor_info1_live_extractor = QRegExp("VISITOR_INFO1_LIVE=([^;]+)");

                if (visitor_info1_live_extractor.indexIn(cookie) != -1) {
                    CurrentTask.VisitorInfo1LiveCookie = visitor_info1_live_extractor.cap(1);

                    UpdateTask(CurrentTask);
                }
            }

            if (DecodeSignature(reply->readAll(), CurrentTask.Signature, &signature)) {
                bool file_valid;

                if (ReopenCurrentFile(&file_valid)) {
                    QNetworkRequest request(QUrl::fromEncoded(QString(CurrentTask.VideoURL + "&signature=" + QUrl::toPercentEncoding(signature)).toAscii()));

                    if (!CurrentTask.VisitorInfo1LiveCookie.isEmpty()) {
                        request.setRawHeader("Cookie", QString("VISITOR_INFO1_LIVE=%1").arg(CurrentTask.VisitorInfo1LiveCookie).toAscii());
                    }
                    if (file_valid) {
                        request.setRawHeader("Range", QString("bytes=%1-").arg(CurrentTask.Done).toAscii());
                    }

                    DownloadReply = NetworkAccessManager->get(request);

                    QObject::connect(DownloadReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
                    QObject::connect(DownloadReply, SIGNAL(readyRead()),                      this, SLOT(downloadDataReady()));
                } else {
                    QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
                }
            } else {
                CurrentTask.State    = YTDownloadState::StateError;
                CurrentTask.ErrorMsg = tr("Cannot decode video signature");

                UpdateTask(CurrentTask);

                QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
            }
        } else {
            if (reply->error() != QNetworkReply::OperationCanceledError) {
                CurrentTask.State    = YTDownloadState::StateError;
                CurrentTask.ErrorMsg = FormatNetworkError(reply->error());

                UpdateTask(CurrentTask);
            }

            QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
        }

        JSPlayerReply = NULL;

        reply->deleteLater();
    } else if (reply == DownloadReply) {
        if (reply->error() == QNetworkReply::NoError) {
            QVariant redirect_target = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

            if (reply->hasRawHeader("Set-Cookie")) {
                QString cookie                       = reply->rawHeader("Set-Cookie");
                QRegExp visitor_info1_live_extractor = QRegExp("VISITOR_INFO1_LIVE=([^;]+)");

                if (visitor_info1_live_extractor.indexIn(cookie) != -1) {
                    CurrentTask.VisitorInfo1LiveCookie = visitor_info1_live_extractor.cap(1);

                    UpdateTask(CurrentTask);
                }
            }

            if (!redirect_target.isNull()) {
                bool file_valid;

                if (ReopenCurrentFile(&file_valid)) {
                    QNetworkRequest request(redirect_target.toUrl());

                    if (!CurrentTask.VisitorInfo1LiveCookie.isEmpty()) {
                        request.setRawHeader("Cookie", QString("VISITOR_INFO1_LIVE=%1").arg(CurrentTask.VisitorInfo1LiveCookie).toAscii());
                    }
                    if (file_valid) {
                        request.setRawHeader("Range", QString("bytes=%1-").arg(CurrentTask.Done).toAscii());
                    }

                    DownloadReply = NetworkAccessManager->get(request);

                    QObject::connect(DownloadReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
                    QObject::connect(DownloadReply, SIGNAL(readyRead()),                      this, SLOT(downloadDataReady()));
                } else {
                    DownloadReply = NULL;

                    QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
                }
            } else {
                QByteArray data = reply->readAll();

                if (data.isEmpty() || CurrentFile.write(data) != -1) {
                    CurrentTask.Done = CurrentTask.Done + data.size();

                    if (CurrentTask.Size == CurrentTask.Done) {
                        CurrentTask.State    = YTDownloadState::StateCompleted;
                    } else {
                        CurrentTask.State    = YTDownloadState::StateError;
                        CurrentTask.Size     = 0;
                        CurrentTask.Done     = 0;
                        CurrentTask.ErrorMsg = tr("Invalid file size, retrying download");
                    }
                } else {
                    CurrentTask.State    = YTDownloadState::StateError;
                    CurrentTask.ErrorMsg = CurrentFile.errorString();
                }

                UpdateTask(CurrentTask);

                CurrentFile.close();

                DownloadReply = NULL;

                QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
            }
        } else {
            if (reply->error() != QNetworkReply::OperationCanceledError) {
                CurrentTask.State    = YTDownloadState::StateError;
                CurrentTask.ErrorMsg = FormatNetworkError(reply->error());

                UpdateTask(CurrentTask);
            }

            CurrentFile.close();

            DownloadReply = NULL;

            QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
        }

        reply->deleteLater();
    }
}

void YTVideoManager::downloadProgress(qint64 received, qint64 total)
{
    Q_UNUSED(received);

    if (CurrentTask.Size == 0) {
        CurrentTask.Size = total;
    }
}

void YTVideoManager::downloadDataReady()
{
    QByteArray data = DownloadReply->readAll();

    if (data.isEmpty() || CurrentFile.write(data) != -1) {
        CurrentTask.Done = CurrentTask.Done + data.size();

        UpdateTask(CurrentTask);
    } else {
        CurrentTask.State    = YTDownloadState::StateError;
        CurrentTask.ErrorMsg = CurrentFile.errorString();

        UpdateTask(CurrentTask);

        DownloadReply->abort();
    }
}

bool YTVideoManager::ReopenCurrentFile(bool *file_valid)
{
    bool                result;
    QIODevice::OpenMode mode;

    if (CurrentFile.isOpen()) {
        CurrentFile.close();
    }

    CurrentFile.setFileName(DestinationDir.path() + QDir::separator() + CurrentTask.VideoId);

    if (CurrentFile.exists() && CurrentFile.size() == CurrentTask.Done) {
        mode        = QIODevice::ReadWrite | QIODevice::Append;
        *file_valid = true;
    } else {
        mode        = QIODevice::ReadWrite | QIODevice::Truncate;
        *file_valid = false;

        CurrentTask.Size = 0;
        CurrentTask.Done = 0;
    }

    if (CurrentFile.open(mode)) {
        CurrentTask.State = YTDownloadState::StateActive;

        result = true;
    } else {
        CurrentTask.State    = YTDownloadState::StateError;
        CurrentTask.ErrorMsg = CurrentFile.errorString();

        result = false;
    }

    UpdateTask(CurrentTask);

    return result;
}

void YTVideoManager::UpdateTask(const YTDownloadTask &task)
{
    for (int i = 0; i < ActiveTasks.size(); i++) {
        if (ActiveTasks.at(i).VideoId == task.VideoId) {
            ActiveTasks.replace(i, task);

            emit taskChanged(task);

            if (TaskDatabase.open()) {
                QSqlQuery query(TaskDatabase);

                query.prepare("UPDATE TASKS SET STATE=:STATE, FMT=:FMT, SIZE=:SIZE, DONE=:DONE, TITLE=:TITLE WHERE VIDEO_ID=:VIDEO_ID");

                query.bindValue(":STATE",    task.State);
                query.bindValue(":FMT",      task.Fmt);
                query.bindValue(":SIZE",     task.Size);
                query.bindValue(":DONE",     task.Done);
                query.bindValue(":TITLE",    task.Title);
                query.bindValue(":VIDEO_ID", task.VideoId);

                query.exec();

                TaskDatabase.close();
            }

            break;
        }
    }
}

bool YTVideoManager::ParseMetadata(const QByteArray &raw_data, QString *video_title, QHash<int, QString> *fmt_url_map, QHash<int, QString> *fmt_sig_map)
{
    QRegExp title_extractor                      = QRegExp("title=([^&]+)",                      Qt::CaseInsensitive);
    QRegExp url_encoded_fmt_stream_map_extractor = QRegExp("url_encoded_fmt_stream_map=([^&]+)", Qt::CaseInsensitive);

    QString data = QString::fromUtf8(raw_data.data());

    if (title_extractor.indexIn(data) != -1) {
        *video_title = QUrl::fromPercentEncoding(title_extractor.cap(1).replace("+", " ").toUtf8());

        if (url_encoded_fmt_stream_map_extractor.indexIn(data) != -1) {
            QStringList splitted = QUrl::fromPercentEncoding(url_encoded_fmt_stream_map_extractor.cap(1).toUtf8()).split(",", QString::SkipEmptyParts);

            for (int i = 0; i < splitted.size(); i++) {
                QRegExp url_extractor  = QRegExp("url=([^&]+)");
                QRegExp itag_extractor = QRegExp("itag=(\\d+)");
                QRegExp sig_extractor  = QRegExp("sig=([^&]+)");
                QRegExp s_extractor_1  = QRegExp("^s=([^&]+)");
                QRegExp s_extractor_2  = QRegExp("&s=([^&]+)");

                if (url_extractor.indexIn(splitted.at(i)) != -1 && itag_extractor.indexIn(splitted.at(i)) != -1) {
                    bool ok  = false;
                    int  fmt = itag_extractor.cap(1).toInt(&ok);

                    if (ok) {
                        QString url            = QUrl::fromPercentEncoding(url_extractor.cap(1).toUtf8());
                        QRegExp signature_mask = QRegExp("signature=[^&]+");

                        if (signature_mask.indexIn(url) == -1) {
                            if (sig_extractor.indexIn(splitted.at(i)) != -1) {
                                url = url + "&signature=" + sig_extractor.cap(1);
                            } else if (s_extractor_1.indexIn(splitted.at(i)) != -1) {
                                (*fmt_sig_map)[fmt] = QUrl::fromPercentEncoding(s_extractor_1.cap(1).toUtf8());
                            } else if (s_extractor_2.indexIn(splitted.at(i)) != -1) {
                                (*fmt_sig_map)[fmt] = QUrl::fromPercentEncoding(s_extractor_2.cap(1).toUtf8());
                            }
                        }

                        (*fmt_url_map)[fmt] = url;
                    }
                }
            }

            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool YTVideoManager::ParseVideoPage(const QByteArray &raw_data, QString *sts, QString *js_player_url)
{
    QRegExp sts_extractor       = QRegExp("\"sts\"\\s*:\\s*(\\d+)");
    QRegExp js_player_extractor = QRegExp("\"assets\"\\s*:.*\"js\"\\s*:\\s*(\"[^\"]+\")");

    QString data = QString::fromUtf8(raw_data.data());

    if (sts_extractor.indexIn(data) != -1) {
        *sts = sts_extractor.cap(1);

        if (js_player_extractor.indexIn(data) != -1) {
            bb::data::JsonDataAccess jda;

            QVariant json_data = jda.loadFromBuffer(QString("{ \"js\" : %1 }").arg(js_player_extractor.cap(1)));

            if (!jda.hasError()) {
                QVariantMap json_map = json_data.value<QVariantMap>();

                if (json_map.contains("js")) {
                    QString url = json_map["js"].toString();

                    if (url.startsWith("//")) {
                        *js_player_url = QString("https:") + url;
                    } else {
                        *js_player_url = url;
                    }
                } else {
                    *js_player_url = "";
                }
            } else {
                *js_player_url = "";
            }
        } else {
            *js_player_url = "";
        }

        return true;
    } else {
        return false;
    }
}

bool YTVideoManager::DecodeSignature(const QByteArray &raw_js_code, const QString &encoded_signature, QString *decoded_signature)
{
    QRegExp js_function_extractor = QRegExp("\\.sig\\s*\\|\\|\\s*([a-zA-Z0-9_\\$]+)\\(");

    QString browser_vars = QString() +
                           "var window = {closed:        false,"    +
                                         "defaultStatus: \"\","     +
                                         "document:      {"         +
                                             "m: 0"                 +
                                         "},"                       +
                                         "frames:        [],"       +
                                         "history:       {"         +
                                             "length: 0"            +
                                         "},"                       +
                                         "innerHeight:   0,"        +
                                         "innerWidth:    0,"        +
                                         "length:        0,"        +
                                         "location:      {"         +
                                             "hash:     \"\","      +
                                             "host:     \"\","      +
                                             "hostname: \"\","      +
                                             "href:     \"\","      +
                                             "origin:   \"\","      +
                                             "pathname: \"\","      +
                                             "port:     0,"         +
                                             "protocol: \"\","      +
                                             "search:   \"\""       +
                                         "},"                       +
                                         "name:          \"\","     +
                                         "navigator:     {"         +
                                             "appCodeName:   \"\"," +
                                             "appName:       \"\"," +
                                             "appVersion:    \"\"," +
                                             "cookieEnabled: true," +
                                             "language:      \"\"," +
                                             "onLine:        true," +
                                             "platform:      \"\"," +
                                             "product:       \"\"," +
                                             "userAgent:     \"\""  +
                                         "},"                       +
                                         "opener:        this,"     +
                                         "outerHeight:   0,"        +
                                         "outerWidth:    0,"        +
                                         "pageXOffset:   0,"        +
                                         "pageYOffset:   0,"        +
                                         "parent:        this,"     +
                                         "screen:        {"         +
                                             "availHeight: 0,"      +
                                             "availWidth:  0,"      +
                                             "colorDepth:  0,"      +
                                             "height:      0,"      +
                                             "pixelDepth:  0,"      +
                                             "width:       0"       +
                                         "},"                       +
                                         "screenLeft:    0,"        +
                                         "screenTop:     0,"        +
                                         "screenX:       0,"        +
                                         "screenY:       0,"        +
                                         "self:          this,"     +
                                         "status:        \"\","     +
                                         "top:           this"      +
                           "};" +
                           "var closed         = window.closed,"        +
                                "defaultStatus = window.defaultStatus," +
                                "document      = window.document,"      +
                                "frames        = window.frames,"        +
                                "history       = window.history,"       +
                                "innerHeight   = window.innerHeight,"   +
                                "innerWidth    = window.innerWidth,"    +
                                "length        = window.length,"        +
                                "location      = window.location,"      +
                                "name          = window.name,"          +
                                "navigator     = window.navigator,"     +
                                "opener        = window.opener,"        +
                                "outerHeight   = window.outerHeight,"   +
                                "outerWidth    = window.outerWidth,"    +
                                "pageXOffset   = window.pageXOffset,"   +
                                "pageYOffset   = window.pageYOffset,"   +
                                "parent        = window.parent,"        +
                                "screen        = window.screen,"        +
                                "screenLeft    = window.screenLeft,"    +
                                "screenTop     = window.screenTop,"     +
                                "screenX       = window.screenX,"       +
                                "screenY       = window.screenY,"       +
                                "self          = window.self,"          +
                                "status        = window.status,"        +
                                "top           = window.top;";

    QString js_code = QString::fromUtf8(raw_js_code.data());

    js_code = js_code.remove(QRegExp("^\\s*\\(\\s*function\\s*\\(\\s*\\)\\s*\\{"));
    js_code = js_code.remove(QRegExp("\\}\\s*\\)\\s*\\(\\s*\\)\\s*;\\s*$"));

    js_code = browser_vars + js_code;

    if (js_function_extractor.indexIn(js_code) != -1) {
        QScriptEngine js_engine;

        QString function_name = js_function_extractor.cap(1);

        js_engine.evaluate(js_code);

        if (!js_engine.hasUncaughtException()) {
            QScriptValue js_function = js_engine.globalObject().property(function_name);

            if (js_function.isFunction()) {
                QScriptValue result = js_function.call(js_engine.globalObject(), QScriptValueList() << encoded_signature);

                if (result.isString()) {
                    *decoded_signature = result.toString();

                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
}

QString YTVideoManager::FormatNetworkError(QNetworkReply::NetworkError error)
{
    switch (error) {
        case QNetworkReply::NoError                           : return QString("HTTP error: No error");
        case QNetworkReply::ConnectionRefusedError            : return QString("HTTP error: Connection refused");
        case QNetworkReply::RemoteHostClosedError             : return QString("HTTP error: Remote host closed connection");
        case QNetworkReply::HostNotFoundError                 : return QString("HTTP error: Host not found");
        case QNetworkReply::TimeoutError                      : return QString("HTTP error: Connection timed out");
        case QNetworkReply::SslHandshakeFailedError           : return QString("HTTP error: SSL handshake failed");
        case QNetworkReply::ProxyConnectionRefusedError       : return QString("HTTP error: Proxy connection refused");
        case QNetworkReply::ProxyConnectionClosedError        : return QString("HTTP error: Proxy connection closed");
        case QNetworkReply::ProxyNotFoundError                : return QString("HTTP error: Proxy not found");
        case QNetworkReply::ProxyTimeoutError                 : return QString("HTTP error: Proxy timeout");
        case QNetworkReply::ProxyAuthenticationRequiredError  : return QString("HTTP error: Proxy required authentication");
        case QNetworkReply::ContentAccessDenied               : return QString("HTTP error: Access denied");
        case QNetworkReply::ContentOperationNotPermittedError : return QString("HTTP error: Operation not permitted");
        case QNetworkReply::ContentNotFoundError              : return QString("HTTP error: Not found");
        case QNetworkReply::AuthenticationRequiredError       : return QString("HTTP error: Authentication required");
        case QNetworkReply::ContentReSendError                : return QString("HTTP error: Content resend error");
        case QNetworkReply::ProtocolUnknownError              : return QString("HTTP error: Unknown protocol");
        case QNetworkReply::ProtocolInvalidOperationError     : return QString("HTTP error: Invalid protocol operation");
        case QNetworkReply::UnknownNetworkError               : return QString("HTTP error: Unknown network error");
        case QNetworkReply::UnknownProxyError                 : return QString("HTTP error: Unknown proxy error");
        case QNetworkReply::UnknownContentError               : return QString("HTTP error: Unknown content error");
        case QNetworkReply::ProtocolFailure                   : return QString("HTTP error: Protocol failure");
        default                                               : return QString("HTTP error: Unknown error");
    }
}
