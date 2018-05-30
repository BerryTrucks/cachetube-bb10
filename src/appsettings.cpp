#include "appsettings.h"

AppSettings::AppSettings(QObject *parent) : QObject(parent)
{
    Settings = new QSettings("Oleg Derevenetz", "CacheTube");
}

AppSettings::~AppSettings()
{
    delete Settings;
}

bool AppSettings::showUnwatchedFirst() const
{
    if (Settings->contains("ShowUnwatchedFirst")) {
        return Settings->value("ShowUnwatchedFirst").toBool();
    } else {
        return true;
    }
}

void AppSettings::setShowUnwatchedFirst(const bool &show)
{
    Settings->setValue("ShowUnwatchedFirst", show);
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

int AppSettings::videoSortOrder() const
{
    if (Settings->contains("VideoSortOrder")) {
        return Settings->value("VideoSortOrder").toInt();
    } else {
        return 0;
    }
}

void AppSettings::setVideoSortOrder(const int &order)
{
    Settings->setValue("VideoSortOrder", order);
}
