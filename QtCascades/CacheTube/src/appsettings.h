#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int  preferredVideoFormat READ preferredVideoFormat WRITE setPreferredVideoFormat)
    Q_PROPERTY(bool autoRepeatPlayback   READ autoRepeatPlayback   WRITE setAutoRepeatPlayback)

public:
    explicit AppSettings(QObject *parent = 0);
    virtual ~AppSettings();

    int  preferredVideoFormat() const;
    void setPreferredVideoFormat(const int &format);

    bool autoRepeatPlayback() const;
    void setAutoRepeatPlayback(const bool &repeat);

private:
    QSettings *Settings;
};

#endif // APPSETTINGS_H
