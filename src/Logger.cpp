#include "Logger.h"

bool CLogger::verbose = false;

void CLogger::generateLogRecord(std::string entry, int priority) {
    time_t t = time(0);
    struct tm *now = localtime(&t);

    char date[80];
    strftime(date, sizeof(date), "%Y-%m-%d_%H:%M:%S", now);

    std::string record = std::string(date) + " | " + entry;

    syslog(priority, record.c_str());
}
