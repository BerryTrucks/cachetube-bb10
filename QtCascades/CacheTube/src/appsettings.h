#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int preferredVideoFormat READ preferredVideoFormat WRITE setPreferredVideoFormat)

public:
    explicit AppSettings(QObject *parent = 0);
    virtual ~AppSettings();

    int  preferredVideoFormat() const;
    void setPreferredVideoFormat(const int &format);

private:
    QSettings *Settings;
};

#endif // APPSETTINGS_H
