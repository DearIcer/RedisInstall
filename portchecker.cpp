#include "portchecker.h"
#include <QProcess>
#include <QRegularExpression>
#include <QDebug>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

PortInfo PortChecker::checkPort(int port)
{
#ifdef Q_OS_WIN
    return checkPortWindows(port);
#else
    return checkPortLinux(port);
#endif
}

#ifdef Q_OS_WIN
PortInfo PortChecker::checkPortWindows(int port)
{
    PortInfo info;
    info.port = port;
    info.isInUse = false;
    info.processId = 0;
    
    PMIB_TCPTABLE_OWNER_PID pTcpTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    
    pTcpTable = (MIB_TCPTABLE_OWNER_PID*)malloc(sizeof(MIB_TCPTABLE_OWNER_PID));
    if (pTcpTable == nullptr) {
        return info;
    }
    
    dwSize = sizeof(MIB_TCPTABLE_OWNER_PID);
    if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET, 
                                        TCP_TABLE_OWNER_PID_ALL, 0)) == ERROR_INSUFFICIENT_BUFFER) {
        free(pTcpTable);
        pTcpTable = (MIB_TCPTABLE_OWNER_PID*)malloc(dwSize);
        if (pTcpTable == nullptr) {
            return info;
        }
    }
    
    if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET,
                                        TCP_TABLE_OWNER_PID_ALL, 0)) == NO_ERROR) {
        for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
            DWORD localPort = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
            if (localPort == port) {
                info.isInUse = true;
                info.processId = pTcpTable->table[i].dwOwningPid;
                
                // Get process name
                HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                if (hProcessSnap != INVALID_HANDLE_VALUE) {
                    PROCESSENTRY32 pe32;
                    pe32.dwSize = sizeof(PROCESSENTRY32);
                    
                    if (Process32First(hProcessSnap, &pe32)) {
                        do {
                            if (pe32.th32ProcessID == info.processId) {
                                info.processName = QString::fromWCharArray(pe32.szExeFile);
                                break;
                            }
                        } while (Process32Next(hProcessSnap, &pe32));
                    }
                    CloseHandle(hProcessSnap);
                }
                break;
            }
        }
    }
    
    free(pTcpTable);
    return info;
}
#else
PortInfo PortChecker::checkPortLinux(int port)
{
    PortInfo info;
    info.port = port;
    info.isInUse = false;
    info.processId = 0;
    
    QProcess process;
    process.start("sh", QStringList() << "-c" << QString("lsof -i :%1 -t").arg(port));
    process.waitForFinished();
    
    QString output = process.readAllStandardOutput().trimmed();
    if (!output.isEmpty()) {
        info.isInUse = true;
        info.processId = output.toInt();
        
        // Get process name
        QProcess nameProcess;
        nameProcess.start("ps", QStringList() << "-p" << QString::number(info.processId) << "-o" << "comm=");
        nameProcess.waitForFinished();
        info.processName = nameProcess.readAllStandardOutput().trimmed();
    }
    
    return info;
}
#endif

QList<PortInfo> PortChecker::getUsedPorts()
{
    QList<PortInfo> ports;
    
#ifdef Q_OS_WIN
    PMIB_TCPTABLE_OWNER_PID pTcpTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    
    pTcpTable = (MIB_TCPTABLE_OWNER_PID*)malloc(sizeof(MIB_TCPTABLE_OWNER_PID));
    if (pTcpTable == nullptr) {
        return ports;
    }
    
    dwSize = sizeof(MIB_TCPTABLE_OWNER_PID);
    if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET,
                                        TCP_TABLE_OWNER_PID_ALL, 0)) == ERROR_INSUFFICIENT_BUFFER) {
        free(pTcpTable);
        pTcpTable = (MIB_TCPTABLE_OWNER_PID*)malloc(dwSize);
        if (pTcpTable == nullptr) {
            return ports;
        }
    }
    
    if ((dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET,
                                        TCP_TABLE_OWNER_PID_ALL, 0)) == NO_ERROR) {
        for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
            PortInfo info;
            info.port = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
            info.isInUse = true;
            info.processId = pTcpTable->table[i].dwOwningPid;
            ports.append(info);
        }
    }
    
    free(pTcpTable);
#else
    QProcess process;
    process.start("sh", QStringList() << "-c" << "netstat -tuln | grep LISTEN");
    process.waitForFinished();
    
    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    for (const QString& line : lines) {
        QRegularExpression re(":([0-9]+)\\s");
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            PortInfo info;
            info.port = match.captured(1).toInt();
            info.isInUse = true;
            ports.append(info);
        }
    }
#endif
    
    return ports;
}
