#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool showUnwatchedFirst   READ showUnwatchedFirst   WRITE setShowUnwatchedFirst)
    Q_PROPERTY(bool autoRepeatPlayback   READ autoRepeatPlayback   WRITE setAutoRepeatPlayback)
    Q_PROPERTY(int  preferredVideoFormat READ preferredVideoFormat WRITE setPreferredVideoFormat)
    Q_PROPERTY(int  videoSortOrder       READ videoSortOrder       WRITE setVideoSortOrder)

public:
    explicit AppSettings(QObject *parent = 0);
    virtual ~AppSettings();

    bool showUnwatchedFirst() const;
    void setShowUnwatchedFirst(const bool &show);

    bool autoRepeatPlayback() const;
    void setAutoRepeatPlayback(const bool &repeat);

    int  preferredVideoFormat() const;
    void setPreferredVideoFormat(const int &format);

    int  videoSortOrder() const;
    void setVideoSortOrder(const int &order);

private:
    QSettings *Settings;
};

#endif // APPSETTINGS_H
