#define PIN_DATA 0      // WiringPi pin 0, BCM pin 17, physical pin 11
#define PIN_LATCH 1     // WiringPi pin 1, BCM pin 18, physical pin 12
#define PIN_SHIFT 2     // WiringPi pin 2, BCM pin 27, physical pin 13
#define PIN_RESET 3     // WiringPi pin 3, BCM pin 22, physical pin 15

#define WAIT_TIME 80000

#define DAEMON_NAME "moreleled"
#define SERVICE_FILE "/etc/init.d/moreleled"
#define PID_FILE "/var/run/moreleled.pid"

#define RED 1
#define BLUE 2
#define YELLOW 4
#define GREEN 8
#define WHITE 16

#define NUMBER_OF_COLORS 5
#define NUMBER_OF_MODULES 5

#include <iostream>
#include <cstring>
#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/sendfile.h>

#include "Resources.h"
#include "Structs.h"
#include "Logger.h"
#include "PinsController.h"
#include "Server.h"
#include "Settings.h"

#include "Logger.cpp"
#include "PinsController.cpp"
#include "Server.cpp"
#include "Settings.cpp"

Resources resources;
CPinsController *pinsController;
CServer *server;

void showHelp() {
    std::cout << resources.help;
}

void signalHandler (int signum) {
    if (signum == SIGUSR1 || signum == SIGUSR2) {
        pinsController->show();
    } else if (signum == SIGINT) {
        std::cout << "\b\b\b  " << std::endl;
        CLogger::generateLogRecord("Użytkownik zatrzymał serwer", LOG_INFO);
        server->stop();
    } else if (signum == SIGTERM) {
        CLogger::generateLogRecord("Zatrzymywanie serwera...", LOG_INFO);
        server->stop();
    } else if (signum == SIGABRT) {
        std::cout << "to lecimy dalej!!" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    int loadConfigResult = CSettings::loadConfig();
    if (loadConfigResult != 0) {
        return loadConfigResult;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            showHelp();
            return 0;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            CLogger::verbose = true;
        }
        else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--install") == 0) {
            if (geteuid() != 0) {
                std::cerr << "Nie jesteś adminem :P" << std::endl;
                return 255;
            }

            std::ofstream serviceFile(SERVICE_FILE, std::ios::trunc);
            chmod(SERVICE_FILE, S_IXGRP | S_IXOTH | S_IRGRP | S_IROTH | S_IRWXU);
            serviceFile << resources.moreleled_service;
            serviceFile.close();
            system("update-rc.d moreleled defaults");

            int source = open("./lampki.bin", O_RDONLY, 0);
            int dest = open("/usr/bin/moreleled", O_WRONLY | O_CREAT, 0755);
            struct stat stat_source;
            fstat(source, &stat_source);
            sendfile(dest, source, 0, stat_source.st_size);

            return 0;
        }
        else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--uninstall") == 0) {
            if (geteuid() != 0) {
                std::cerr << "Nie jesteś adminem :P" << std::endl;
                return 255;
            }

            remove(SERVICE_FILE);
            system("update-rc.d moreleled remove");
            return 0;
        }
        else if (strcmp(argv[i], "--kill") == 0) {
            std::ifstream pidFile(PID_FILE, std::ios::in);
            pid_t pid;
            pidFile >> pid;
            kill(pid, 15);
            return 0;
        }
        else if (memcmp(argv[i], "-p", sizeof("-p") - 1) == 0) {
            int p = 0;
            if (argv[i][2]) {
                p = atoi(argv[i] + 2);
            } else if (i != argc - 1) {
                std::cout << argv[i + 1] << std::endl;
                p = atoi(argv[i + 1]);
                i++;
            }

            std::cout << p << std::endl;

            if (p <= 0 || p > 65535) {
                std::cerr << "Podany port poza zakresem (1 - 65535): " << p << std::endl;
                return 251;
            } else {
                CSettings::port = p;
            }
        }
        else {
            std::cerr << "Nieznana opcja: " << argv[i] << std::endl;
            return 250;
        }
    }

    if (geteuid() != 0) {
        std::cerr << "Wymagane uprawnienia roota\nCzy chodziło Ci o: sudo " << argv[0] << std::endl;
        return 255;
    }

    if (wiringPiSetup() < 0) {
        std::cerr << "Nie można zainicjalizować biblioteki wiringPi." << std::endl;
        return 250;
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    signal(SIGABRT, signalHandler);

    pinsController = new CPinsController;
    server = new CServer(pinsController, resources);

    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);

    if (!CLogger::verbose) {
        pid_t pid, sid;
        pid = fork();

        if (pid < 0) { syslog(LOG_ERR, "Błąd przy fork()");return 1; }
        if (pid > 0) {
            std::ofstream pidFile;
            pidFile.open(PID_FILE, std::ios::trunc);
            pidFile << pid;
            pidFile.close();
            syslog(LOG_INFO, "Uruchamianie demona");
            return 0;
        }

        umask(0);

        sid = setsid();
        if (sid < 0) { syslog(LOG_INFO, "Błąd przy sid()");return 2; }
        if ((chdir("/")) < 0) { syslog(LOG_INFO, "Błąd przy zmianie katalogu");return 3; }

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    int serverRes = server->start();

    closelog();
    remove(PID_FILE);
    return serverRes;
}
