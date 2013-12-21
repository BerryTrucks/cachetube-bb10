#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/QTimer>
#include <QtSql/QSqlQuery>
#include <QtNetwork/QNetworkRequest>

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
                    task.VisitorInfo1LiveCookie = "";

                    if (task.State == YTDownloadState::StateActive) {
                        task.State = YTDownloadState::StateQueued;
                    }

                    DownloadTasks.append(task);
                }
            }
        }

        TaskDatabase.close();
    }

    PreferredVideoFormat = preferred_format;
    NetworkAccessManager = network_access_manager;
    MetadataReply        = NULL;
    DownloadReply        = NULL;

    QObject::connect(NetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkRequestFinished(QNetworkReply*)));

    QTimer::singleShot(QUEUE_RUN_AFTER, this, SLOT(runQueue()));
}

YTVideoManager::~YTVideoManager()
{
    for (int i = 0; i < DeletedTasks.size(); i++) {
        QFile::remove(DestinationDir.path() + QDir::separator() + DeletedTasks.at(i).VideoId);
    }

    if (MetadataReply != NULL) {
        delete MetadataReply;
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
        for (int i = 0; i < DownloadTasks.size(); i++) {
            if (DownloadTasks.at(i).VideoId == video_id) {
                return false;
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
        task.VisitorInfo1LiveCookie = "";

        DownloadTasks.append(task);

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
    for (int i = 0; i < DownloadTasks.size(); i++) {
        if (DownloadTasks.at(i).VideoId == video_id) {
            for (int j = 0; j < DeletedTasks.size(); j++) {
                if (DeletedTasks.at(j).VideoId == video_id) {
                    QFile::remove(DestinationDir.path() + QDir::separator() + DeletedTasks.at(j).VideoId);

                    DeletedTasks.removeAt(j);

                    break;
                }
            }

            if (CurrentTask.VideoId == video_id) {
                if (MetadataReply != NULL) {
                    MetadataReply->abort();
                }
                if (DownloadReply != NULL) {
                    DownloadReply->abort();
                }
            }

            YTDownloadTask task = DownloadTasks.at(i);

            DeletedTasks.append(task);

            DownloadTasks.removeAt(i);

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
            for (int j = 0; j < DownloadTasks.size(); j++) {
                if (DownloadTasks.at(j).VideoId == video_id) {
                    return false;
                }
            }

            YTDownloadTask task = DeletedTasks.at(i);

            if (task.State == YTDownloadState::StateActive) {
                task.State = YTDownloadState::StateQueued;
            }

            DownloadTasks.append(task);

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
    for (int i = 0; i < DownloadTasks.size(); i++) {
        if (DownloadTasks.at(i).VideoId == video_id && DownloadTasks.at(i).State != YTDownloadState::StateCompleted &&
                                                       DownloadTasks.at(i).State != YTDownloadState::StatePaused) {
            if (CurrentTask.VideoId == video_id) {
                if (MetadataReply != NULL) {
                    MetadataReply->abort();
                }
                if (DownloadReply != NULL) {
                    DownloadReply->abort();
                }
            }

            YTDownloadTask task = DownloadTasks.at(i);

            task.State = YTDownloadState::StatePaused;

            DownloadTasks.replace(i, task);

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
    for (int i = 0; i < DownloadTasks.size(); i++) {
        if (DownloadTasks.at(i).VideoId == video_id && DownloadTasks.at(i).State == YTDownloadState::StatePaused) {
            YTDownloadTask task = DownloadTasks.at(i);

            task.State = YTDownloadState::StateQueued;

            DownloadTasks.replace(i, task);

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
    for (int i = 0; i < DownloadTasks.size(); i++) {
        if (DownloadTasks.at(i).VideoId == video_id) {
            return QUrl::fromLocalFile(DestinationDir.path() + QDir::separator() + DownloadTasks.at(i).VideoId).toString();
        }
    }

    return "";
}

QList<YTDownloadTask> YTVideoManager::getTaskList()
{
    return DownloadTasks;
}

void YTVideoManager::runQueue()
{
    if (MetadataReply == NULL && DownloadReply == NULL) {
        for (int i = 0; i < DownloadTasks.size(); i++) {
            if (DownloadTasks.at(i).State != YTDownloadState::StateCompleted && DownloadTasks.at(i).State != YTDownloadState::StatePaused) {
                CurrentTask = DownloadTasks.at(i);

                QNetworkRequest request(QUrl::fromEncoded(QString("http://www.youtube.com/get_video_info?&video_id=%1&el=detailpage&ps=default&eurl=&gl=US&hl=en").arg(CurrentTask.VideoId).toAscii()));

                request.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);

                MetadataReply = NetworkAccessManager->get(request);

                break;
            }
        }
    }
}

void YTVideoManager::networkRequestFinished(QNetworkReply *reply)
{
    if (reply == MetadataReply) {
        if (reply->error() == QNetworkReply::NoError) {
            QString             video_title;
            QHash<int, QString> fmt_url_map;

            if (reply->hasRawHeader("Set-Cookie")) {
                QString cookie                       = reply->rawHeader("Set-Cookie");
                QRegExp visitor_info1_live_extractor = QRegExp("VISITOR_INFO1_LIVE=([^;]+)");

                if (visitor_info1_live_extractor.indexIn(cookie) != -1) {
                    CurrentTask.VisitorInfo1LiveCookie = visitor_info1_live_extractor.cap(1);

                    UpdateTask(CurrentTask);
                }
            }

            if (ParseMetadata(reply->readAll(), &video_title, &fmt_url_map)) {
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
    for (int i = 0; i < DownloadTasks.size(); i++) {
        if (DownloadTasks.at(i).VideoId == task.VideoId) {
            DownloadTasks.replace(i, task);

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

bool YTVideoManager::ParseMetadata(const QByteArray &raw_data, QString *video_title, QHash<int, QString> *fmt_url_map)
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

                if (url_extractor.indexIn(splitted.at(i)) != -1 && itag_extractor.indexIn(splitted.at(i)) != -1) {
                    bool ok  = false;
                    int  fmt = itag_extractor.cap(1).toInt(&ok);

                    if (ok) {
                        QString url            = QUrl::fromPercentEncoding(url_extractor.cap(1).toUtf8());
                        QRegExp signature_mask = QRegExp("signature=[^&]+");

                        if (signature_mask.indexIn(url) == -1) {
                            if (sig_extractor.indexIn(splitted.at(i)) != -1) {
                                url = url + "&signature=" + sig_extractor.cap(1);
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
