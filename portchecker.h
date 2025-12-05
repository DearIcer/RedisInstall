#ifndef PORTCHECKER_H
#define PORTCHECKER_H

#include <QString>
#include <QList>

struct PortInfo
{
    int port;
    bool isInUse;
    QString processName;
    int processId;
};

class PortChecker
{
public:
    static PortInfo checkPort(int port);
    static QList<PortInfo> getUsedPorts();
    
private:
#ifdef Q_OS_WIN
    static PortInfo checkPortWindows(int port);
#else
    static PortInfo checkPortLinux(int port);
#endif
};

#endif // PORTCHECKER_H
