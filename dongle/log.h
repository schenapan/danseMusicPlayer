/****************************************************************************
**
** This file is writed by Legrand and can be use by Legrand
** for this project or other projects.
**
$Date:
$Rev:
$Author:
$HeadURL:
$Id:
**
**
******************************************************************************/

#ifndef __LOG_H__
#define __LOG_H__

#include <iomanip>
#include <sstream>
#include <iostream>

#include <QDateTime>
#include <QDebug>

//---------------------------------------------------------------------------
// A simple C++ logger
// Taken and tweaked from : http://www.drdobbs.com/cpp/logging-in-c/201804215
//
// No log.cpp associated. Simply include this header in your source file.
// Usage : LOGGER(logINFO) << "some text " << "some more text";
//---------------------------------------------------------------------------

/// defini le formattage a appliquer aux QString pour les afficher sur les flux de sorties
inline std::ostream & operator<< (std::ostream &out, const QString &s)
{
    out << s.toStdString();
    return out;
}

inline std::string NowTime();

enum TLogLevel {logFATAL, logERROR, logWARNING, logINFO, logDEBUG, logTRACE};

class Log
{
public:
    Log();
    virtual ~Log();
    std::ostringstream& Get(TLogLevel level, const char *file, unsigned long line);
public:
    static TLogLevel& ReportingLevel();
    static std::string ToString(TLogLevel level);
protected:
    std::ostringstream os;
private:
    Log(const Log&);
    Log& operator =(const Log&);
};

inline Log::Log()
{
}

inline std::string Log::ToString(TLogLevel level)
{
    static const char* const buffer[] = {"FATAL", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE"};
    return buffer[level];
}

inline std::ostringstream& Log::Get(TLogLevel level, const char *file, unsigned long line)
{
    os << NowTime() << " - ";
    // file est un chemin entier du style ..\\modem\\uart.cpp
    // on veut afficher "uart.cpp(377)"
    os << (QString("%1(%2)").arg(file).arg(line)).section('\\', -1).toStdString();
    // on veut le numero de ligne entre parenthese
    // le numero est fixe a 3 chiffres
    os << " - " << ToString(level) << " - ";
    return os;
}

inline Log::~Log()
{
    // bug : le debugger de Qt creator ne permet pas de voir le texte affiche sur stdout/stderr
    // alors on utilise les sorties qDebug() ou qFatal()
    qDebug() << os.str().c_str();

    //force crash on fatal error
    if(ReportingLevel() == logFATAL)
        abort();
}

inline TLogLevel& Log::ReportingLevel()
{
    static TLogLevel reportingLevel = logTRACE;
    return reportingLevel;
}

typedef Log FILELog;

#define LOGGER(level) \
    if (level > FILELog::ReportingLevel()) ; \
    else Log().Get(level, __FILE__, __LINE__)

#define LOGENTRY \
    if (logTRACE > FILELog::ReportingLevel()) ; \
    else Log().Get(logTRACE, __FILE__, __LINE__) << "entering " << __FUNCTION__ << "() "

#define LOGEXIT \
    if (logTRACE > FILELog::ReportingLevel()) ; \
    else Log().Get(logTRACE, __FILE__, __LINE__) << "leaving " <<__FUNCTION__ << "() "

inline std::string NowTime()
{
    return QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toStdString();
}

#endif //__LOG_H__
