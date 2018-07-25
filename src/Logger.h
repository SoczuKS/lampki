#ifndef _LOGGER_H
#define _LOGGER_H

#include <ctime>

#include "Settings.h"

class CLogger {
public:
    CLogger(){};

    static void generateLogRecord(std::string entry, int priority);

    static bool verbose;
};
#endif
