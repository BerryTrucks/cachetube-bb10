#ifndef CACHETUBE_HPP
#define CACHETUBE_HPP

#include <QtCore/QObject>

namespace bb { namespace cascades { class Application; }}

class CacheTube : public QObject
{
    Q_OBJECT
public:
    CacheTube(bb::cascades::Application *app);
    virtual ~CacheTube() {}
};

#endif // CACHETUBE_HPP
