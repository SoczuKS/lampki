#ifndef _SETTINGS_H
#define _SETTINGS_H

#define CONFIG_FILE "/etc/moreleled.conf"

#include <fstream>
#include <string>

class CSettings {
public:
    CSettings(){};

    static unsigned short int port;
    static std::vector<std::string> allowedOrigins;
    static std::vector<std::string> bannedIPs;

    static int loadConfig();


    enum sections{ NONE, ALLOWED_DOMAINS, PORT, BANNED_IPS };

private:
    static inline bool checkSection(char* buf, std::string req);
};
#endif
