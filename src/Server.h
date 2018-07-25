#ifndef _SERVER_H
#define _SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <sys/stat.h>
#include <vector>
#include <bitset>

#include "Structs.h"
#include "PinsController.h"
#include "Resources.h"
#include "Logger.h"
#include "Settings.h"

class CServer {
public:
    CServer(CPinsController *pc, Resources res);
    ~CServer();
    int start();
    inline bool checkParameter(char* buf, std::string req);
    void logRequest(std::string request, std::string ip);
    void stop();

private:
    bool running = true;
    bool isOriginAllowed(std::string origin);
    bool isIPBanned(std::string ip);
    int server;
    int connection;
    CPinsController *pinsController;
    Resources resources;
};
#endif
