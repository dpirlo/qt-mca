#ifndef APEXCEPTIONS_H
#define APEXCEPTIONS_H
#include <QException>
#include <QDebug>

namespace ap {

    class Exceptions: public QException
    {
    public:
        Exceptions(const char *desc);
        const char *excdesc;

    };
}

#endif // APEXCEPTIONS_H
