#include "Settings.h"

unsigned short int CSettings::port = 8080;
std::vector<std::string> CSettings::allowedOrigins = std::vector<std::string>();
std::vector<std::string> CSettings::bannedIPs = std::vector<std::string>();

int CSettings::loadConfig() {
    allowedOrigins.clear();
    bannedIPs.clear();
    auto currentSection = sections::NONE;

    std::ifstream confFile(CONFIG_FILE, std::ios::in);

    if (!confFile.good()) {
        CLogger::generateLogRecord("Config file not exist", LOG_DEBUG);
        confFile.close();
        return 0;
    }

    std::string line;
    while (std::getline(confFile, line)) {
        if (line[0] == '[') {
            if (checkSection(strdup(line.c_str()), "[allowedDomains]")) {
                currentSection = sections::ALLOWED_DOMAINS;
            }
            else if (checkSection(strdup(line.c_str()), "[port]")) {
                currentSection = sections::PORT;
            }
            else if (checkSection(strdup(line.c_str()), "[bannedIPs]")) {
                currentSection = sections::BANNED_IPS;
            }
        } else {
            switch(currentSection) {
                case ALLOWED_DOMAINS:
                    allowedOrigins.push_back(line);
                    break;

                case PORT:
                    port = stoi(line);
                    if (port <= 0 || port > 65535) {
                        std::cerr << "Port zdefiniowany w pliku konfiguracyjnym znajduje się poza dozwolonym zakresem (1 - 65535): " << port << std::endl;
                        confFile.close();
                        return 252;
                    }
                    break;

                case BANNED_IPS:
                    bannedIPs.push_back(line);
                    break;

                case NONE: default:
                    break;
            }
        }
    }
    confFile.close();
    return 0;
}

inline bool CSettings::checkSection(char* buf, std::string req) {
    return (memcmp(buf, req.c_str(), strlen(req.c_str())) == 0);
}
