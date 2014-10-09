#ifndef INVOCATIONHELPER_H
#define INVOCATIONHELPER_H

#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <bb/system/InvokeRequest>
#include <bb/system/InvokeManager>

class InvocationHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl videoUrl READ videoUrl)

public:
    explicit InvocationHelper(QObject *parent = 0);
    virtual ~InvocationHelper();

    QUrl videoUrl() const;

private slots:
    void invoked(const bb::system::InvokeRequest &request);

signals:
    void invocationStarted();

private:
    QUrl                      VideoURL;
    bb::system::InvokeManager *InvocationManager;
};

#endif // INVOCATIONHELPER_H
