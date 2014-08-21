#include "appsettings.h"

AppSettings::AppSettings(QObject *parent) : QObject(parent)
{
    Settings = new QSettings("Oleg Derevenetz", "CacheTube");
}

AppSettings::~AppSettings()
{
    delete Settings;
}

int AppSettings::preferredVideoFormat() const
{
    if (Settings->contains("PreferredVideoFormat")) {
        return Settings->value("PreferredVideoFormat").toInt();
    } else {
        return 0;
    }
}

void AppSettings::setPreferredVideoFormat(const int &format)
{
    Settings->setValue("PreferredVideoFormat", format);
}

bool AppSettings::autoRepeatPlayback() const
{
    if (Settings->contains("AutoRepeatPlayback")) {
        return Settings->value("AutoRepeatPlayback").toBool();
    } else {
        return false;
    }
}

void AppSettings::setAutoRepeatPlayback(const bool &repeat)
{
    Settings->setValue("AutoRepeatPlayback", repeat);
}
