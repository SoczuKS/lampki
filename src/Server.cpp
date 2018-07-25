#include "Server.h"

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

CServer::CServer(CPinsController *pc, Resources res) {
    pinsController = pc;
    resources = res;
}

CServer::~CServer() {
    pinsController->reset();
}

int CServer::start() {
    int bufsize = 4096;
    char buffer[bufsize] {0};
    struct sockaddr_in server_addr;
    socklen_t size;
    std::vector<LEDS> goodLEDS;
    std::vector<LEDS> badLEDS;

    CLogger::generateLogRecord("Serwer startuje na *:" + std::to_string(CSettings::port), LOG_INFO);

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0) {
        CLogger::generateLogRecord("Socket nie został utworzony", LOG_ERR);
        return 4;
    }

    int enable = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        CLogger::generateLogRecord("Niepowodzenie ustawiania SO_REUSEADDR", LOG_WARNING);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(CSettings::port);


    if ((bind(server, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) {
        CLogger::generateLogRecord("Port " + std::to_string(CSettings::port) + " zajęty, użyj polecenia netstat, aby zlokalizować problem", LOG_ERR);
        return 5;
    }

    size = sizeof(server_addr);

    CLogger::generateLogRecord("Serwer działa", LOG_INFO);
    pinsController = new CPinsController;
    pinsController->welcome();

    listen(server, 10);

    while (running) {
        responseStruct response;
        connection = accept(server,(struct sockaddr*)&server_addr,&size);

        if (connection < 0) {
            if (running) {
                CLogger::generateLogRecord("Bład przy akceptowaniu połączeniem", LOG_NOTICE);
                continue;
            }
            break;
        }

        recv(connection, buffer, bufsize, 0);

        int c = 0;          // numer przetwarzanego znaku w buforze
        int sektor {-1}, kolor {-1}, stan {-1};

        while (buffer[c++] != '\r');
        buffer[c] = '\n';
        buffer[c+1] = '\0';

        c += 2;         // skacze na początek drugiej linii :D

        std::string origin{""};
        while (!checkParameter(buffer + c, "\r\n\r\n")) {
            if (checkParameter(buffer + c, "Origin: ")) {
                int orStart = c + 8;        // 8 is length of "Origin: "
                while(buffer[c++] != '\r');
                int orLen = c - orStart - 1;

                if (checkParameter(buffer + orStart, "http://")) {
                    orStart += 7;
                    orLen -= 7;
                } else if (checkParameter(buffer + orStart, "https://")) {
                    orStart += 8;
                    orLen -= 8;
                }
                origin = std::string(buffer + orStart, orLen);
            } else {
                c++;
            }
        }

        CLogger::generateLogRecord(std::string(inet_ntoa(server_addr.sin_addr)) + " | " + std::string(buffer), LOG_DEBUG);

        if (isIPBanned(std::string(inet_ntoa(server_addr.sin_addr)))) {
            response.responseCode = "401 Unauthorized";
            response.contentType = "application/json;charset=utf-8";
            response.responseContent = "{\"errorCode\":5}";
            auto resp = response.generateResponse();
            send(connection, resp.data(), resp.size(), 0);
            close(connection);
            continue;
        }

        if (!isOriginAllowed(origin)) {
            response.responseCode = "401 Unauthorized";
            response.contentType = "application/json;charset=utf-8";
            response.responseContent = "{\"errorCode\":4}";
            auto resp = response.generateResponse();
            send(connection, resp.data(), resp.size(), 0);
            close(connection);
            continue;
        }

        c = 0;
        bool isRequestCorrect = true;
        while (buffer[c++] != '/' && c <= bufsize);
        if (checkParameter(buffer + c, "reset/")) {
            c += 6;                 // długość "reset/"
            int kolor = atoi(buffer + c++);
            int r = pinsController->resetSingleColor(kolor);

            response.responseCode = "200 OK";
            response.contentType = "application/json;charset=utf-8";
            response.responseContent = "{\"errorCode\":" + std::to_string(r) + "}";
        }
        else if (checkParameter(buffer + c, "reset")) {
            pinsController->reset();
            response.responseCode = "200 OK";
            response.contentType = "application/json;charset=utf-8";
            response.responseContent = "{\"errorCode\":0}";
        }
        else if (checkParameter(buffer + c, "pinout")) {
            response.responseCode = "200 OK";
            response.contentType = "image/png";
            response.responseContent = resources.pinout_png;
        }
        else if (checkParameter(buffer + c, "dump")) {
            auto dumpResult = pinsController->dump();
            response.responseCode = "200 OK";
            response.contentType = "text/plain;charset=utf-8";
            response.responseContent = dumpResult;
        }
        else if (checkParameter(buffer + c, "show")) {
            pinsController->show();
            response.responseCode = "200 OK";
            response.contentType = "application/json;charset=utf-8";
            response.responseContent = "{\"errorCode\":0}";
        }
        else if (checkParameter(buffer + c, "quit")) {
            running = false;
            pinsController->reset();
            response.responseCode = "200 OK";
            response.contentType = "application/json;charset=utf-8";
            response.responseContent = "{\"errorCode\":0}";
            CLogger::generateLogRecord("Serwer zatrzymany przez żądanie HTTP", LOG_INFO);
        }
        else if (checkParameter(buffer + c, "favicon.ico")) {
            response.responseCode = "200 OK";
            response.contentType = "image/x-icon";
            response.responseContent = resources.favicon_ico;
        }
        else if (checkParameter(buffer + c, "readme")) {
            response.responseCode = "200 OK";
            response.contentType = "text/html;charset=utf-8";
            response.responseContent = resources.readme_html;
        }
        else if (checkParameter(buffer + c, "readme_src")) {
            response.responseCode = "200 OK";
            response.contentType = "text/plain;charset=utf-8";
            response.responseContent = resources.readme_src;
        }
        else if (checkParameter(buffer + c, "favicon.png")) {
            response.responseCode = "200 OK";
            response.contentType = "image/png";
            response.responseContent = resources.favicon_png;
        }
        else if (checkParameter(buffer + c, " ")) {
            response.responseCode = "200 OK";
            response.contentType = "text/html";
            response.responseContent = resources.index_html;
        }
        else if (checkParameter(buffer + c, "state")) {
            response.responseCode = "200 OK";
            response.contentType = "application/json";
            response.responseContent = pinsController->generateStateJSON();
        }
        else if (checkParameter(buffer + c, "test")) {
            pinsController->test();
            response.responseCode = "200 OK";
            response.contentType = "application/json";
            response.responseContent = "{\"errorCode\":0}";
        }
        else if (checkParameter(buffer + c, "997")) {
            pinsController->police();
            response.responseCode = "200 OK";
            response.contentType = "application/json";
            response.responseContent = "{\"errorCode\":0}";
        }
        else if (checkParameter(buffer + c, "welcome")) {
            pinsController->welcome();
            response.responseCode = "200 OK";
            response.contentType = "application/json";
            response.responseContent = "{\"errorCode\":0}";
        }
        else if (checkParameter(buffer + c, "rand")) {
            pinsController->randomize();
            response.responseCode = "200 OK";
            response.contentType = "application/json";
            response.responseContent = "{\"errorCode\":0}";
        }
        else if (checkParameter(buffer + c, "xmas")) {
            pinsController->xmas();
            response.responseCode = "200 OK";
            response.contentType = "application/json";
            response.responseContent = "{\"errorCode\":0}";
        }
        else {
            do {
                sektor = atoi(buffer + c++);

                while (buffer[c++] != '_' && c <= bufsize);
                if (c > bufsize) {
                    isRequestCorrect = false;
                    break;
                }
                kolor = atoi(buffer + c++);

                while (buffer[c++] != '_' && c <= bufsize);
                if (c > bufsize) {
                    isRequestCorrect = false;
                    break;
                }
                stan = atoi(buffer + c++);

                if (c > bufsize) {
                    isRequestCorrect = false;
                    break;
                }

                int r = pinsController->setLED(sektor, kolor, stan);
                if (r == 0) {
                    goodLEDS.push_back(LEDS(sektor, kolor, stan));
                } else {
                    badLEDS.push_back(LEDS(sektor, kolor, stan, r));
                }

                while (buffer[c] != '&' && buffer[c] != ' ') {
                    c++;
                    if (c > bufsize) {
                        isRequestCorrect = false;
                        break;
                    }
                }
            } while (buffer[c++] != ' ' && c <= bufsize);

            if (isRequestCorrect) {
                std::string json = "{\"good\":[";

                if (goodLEDS.size() > 0) {
                    json += goodLEDS[0].toJSON();
                    for (unsigned int i = 1; i < goodLEDS.size(); i++) {
                        json += ",";
                        json += goodLEDS[i].toJSON();
                    }
                }

                json += "],\"bad\":[";

                if (badLEDS.size() > 0) {
                    json += badLEDS[0].toJSON();
                    for (unsigned int i = 1; i < badLEDS.size(); i++) {
                        json += ",";
                        json += badLEDS[i].toJSON();
                    }
                }

                json += "]}";

                response.responseCode = "200 OK";
                response.contentType = "application/json;charset=utf-8";
                response.responseContent = json;


                if (goodLEDS.size() > 0) {
                    pinsController->updateState();
                }

                goodLEDS.clear();
                badLEDS.clear();
            } else {
                response.responseCode = "400 Bad Request";
                response.contentType = "application/json;charset=utf-8";
                response.responseContent = "{\"errorCode\":6}";
            }
        }

        auto resp = response.generateResponse();
        send(connection, resp.data(), resp.size(), 0);
        close(connection);

    }

    return 0;
}

inline bool CServer::checkParameter(char* buf, std::string req) {
    return (memcmp(buf, req.c_str(), strlen(req.c_str())) == 0);
}

bool CServer::isOriginAllowed(std::string origin) {
    if (CSettings::allowedOrigins.empty()) {
        return true;
    }
    if (origin == "") {
        return true;
    }

    for (unsigned int i = 0; i < CSettings::allowedOrigins.size(); i++) {
        if (origin == CSettings::allowedOrigins[i]) {
            return true;
        }
    }
    CLogger::generateLogRecord("Unauthorized origin: " + origin, LOG_WARNING);
    return false;
}

bool CServer::isIPBanned(std::string ip) {
    if (CSettings::bannedIPs.empty()) {
        return false;
    }

    for (unsigned int i = 0; i < CSettings::bannedIPs.size(); i++) {
        if (ip == CSettings::bannedIPs[i]) {
            return true;
        }
    }
    CLogger::generateLogRecord("Banned IP: " + ip, LOG_WARNING);
    return false;
}

void CServer::stop() {
    running = false;
    pinsController->show();
    pinsController->reset();
    close(connection);
    close(server);
}
