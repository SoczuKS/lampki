#ifndef _STRUCTS_H
#define _STRUCTS_H

#include <wiringPi.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <csignal>
#include <map>
#include <fstream>
#include <ctime>
#include <sys/stat.h>
#include <vector>
#include <bitset>

struct responseStruct {
    std::string responseCode = "400 Bad Request";
    int age = 0;
    std::string contentType = "text/plain;charset=utf-8";
    std::string server = "MoreleLed/6.6.6";
    std::string accessControlAllowOrigin = "*";
    std::string responseContent = "";

    std::string generateResponse() {
        std::string resp = "HTTP/1.0 " + responseCode + "\r\n";
        resp += "Age: " + std::to_string(age) + "\r\n";
        resp += "Content-Type: " + contentType + "\r\n";
        resp += "Server: " + server + "\r\n";
        resp += "Access-Control-Allow-Origin: " + accessControlAllowOrigin + "\r\n\r\n";
        resp += responseContent;

        return resp;
    }

    std::string generateHeader() {
        std::string resp = "HTTP/1.0 " + responseCode + "\r\n";
        resp += "Age: " + std::to_string(age) + "\r\n";
        resp += "Content-Type: " + contentType + "\r\n";
        resp += "Server: " + server + "\r\n";
        resp += "Access-Control-Allow-Origin: " + accessControlAllowOrigin + "\r\n\r\n";

        return resp;
    }
};

struct LEDS {
    LEDS(int Sektor, int Kolor, int Stan, int E = 0) : sektor(Sektor), kolor(Kolor), stan(Stan), e(E) {}
    int sektor;
    int kolor;
    int stan;
    int e;
    std::string toJSON() {
        std::string json {"{"};
        json += "\"sektor\":" + std::to_string(sektor) + ",";
        json += "\"kolor\":" + std::to_string(kolor) + ",";
        json += "\"stan\":" + std::to_string(stan);
        if (e != 0) {
            json += ",\"error\":" + std::to_string(e);
        }
        json += "}";
        return json;
    }

    std::string toStateJSON() {
        std::string json {"{"};
        json += "\"sektor\":" + std::to_string(sektor) + ",";
        json += "\"kolor\":" + std::to_string(kolor);
        json += "}";
        return json;
    }
};
#endif
