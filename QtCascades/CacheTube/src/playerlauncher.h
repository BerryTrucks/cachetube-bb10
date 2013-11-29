#ifndef PLAYERLAUNCHER_H
#define PLAYERLAUNCHER_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include <bb/cascades/CustomControl>

class PlayerLauncher : public bb::cascades::CustomControl
{
    Q_OBJECT

public:
    explicit PlayerLauncher(QObject *parent = 0);
    virtual ~PlayerLauncher();

    Q_INVOKABLE bool launchPlayer(const QString &video_uri, const QString &video_mime_type, const QString &video_title);

public slots:
    void invokeFinished();
};

#endif // PLAYERLAUNCHER_H
