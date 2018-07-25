#include "PinsController.h"

#include <wiringPi.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <map>
#include <vector>
#include <bitset>

#include "Structs.h"

CPinsController::CPinsController() {
    pinMode(PIN_DATA, OUTPUT);
    pinMode(PIN_LATCH, OUTPUT);
    pinMode(PIN_SHIFT, OUTPUT);
    pinMode(PIN_RESET, OUTPUT);

    digitalWrite(PIN_RESET, LOW);
    usleep(WAIT_TIME);
    digitalWrite(PIN_RESET, HIGH);
}

std::string CPinsController::generateStateJSON() {
    std::vector<LEDS> turnedOnLEDS;
    std::string json = "{\"settings\":{\"sektory\":" + std::to_string(NUMBER_OF_MODULES) + ",\"kolory\":" + std::to_string(NUMBER_OF_COLORS) + "},\"leds\":[";

    for (int i = 0; i < NUMBER_OF_MODULES; i++) {
        auto s = 0x01;

        for (int j = 0; j < 8; j++) {
            if ((currentLEDS[i] & s) == s) {
                turnedOnLEDS.push_back(LEDS(i, j, 1));
            }
            s <<= 1;
        }
    }

    if (turnedOnLEDS.size() > 0) {
        json += turnedOnLEDS[0].toStateJSON();
        for (unsigned int i = 1; i < turnedOnLEDS.size(); i++) {
            json += ",";
            json += turnedOnLEDS[i].toStateJSON();
        }
    }

    json += "]}";
    return json;
}

void CPinsController::show() {
    std::cout << "---- LEDS ----\n";
    for (int i = 0; i < NUMBER_OF_MODULES; i++) {
        auto s = 0x80;          // największy bit (128)

        for (int j = 0; j < 8; j++) {
            if ((currentLEDS[i] & s) == s) {
                std::cout << "#";
            } else {
                std::cout << ".";
            }
            s >>= 1;
        }
        std::cout << '\n';
    }
}

int CPinsController::setLED(int sektor, int kolor, int stan) {
    if (sektor < 0 || sektor >= NUMBER_OF_MODULES) {      // indeks sektora
        return 1;
    }

    if (kolor < 0 || kolor >= NUMBER_OF_COLORS) {         // indeks koloru (0 - 4)
        return 2;
    }

    if (stan == 1) {                                       // włączanie lam,pek
        currentLEDS[sektor] = (currentLEDS[sektor] | colorsDefinition[kolor]);
    } else if (stan == 0) {                             // wyłączanie lampek
        currentLEDS[sektor] = (currentLEDS[sektor] & ~colorsDefinition[kolor]);
    } else {
        return 3;
    }

    return 0;
}

void CPinsController::reset() {
    if (easterEggThread.joinable()) {
        easterEggRunning = false;
        easterEggThread.join();
        currentEasterEgg = NONE;
    }

    for (int i = 0; i < NUMBER_OF_MODULES; i++) {
        currentLEDS[i] = 0;
    }

    digitalWrite(PIN_RESET, LOW);
    usleep(WAIT_TIME);
    digitalWrite(PIN_RESET, HIGH);
    usleep(WAIT_TIME);digitalWrite(PIN_RESET, LOW);
    usleep(WAIT_TIME);
    digitalWrite(PIN_RESET, HIGH);digitalWrite(PIN_RESET, LOW);
    usleep(WAIT_TIME);
    digitalWrite(PIN_RESET, HIGH);
}

int CPinsController::resetSingleColor(int kolor) {
    if (kolor < 0 || kolor >= NUMBER_OF_COLORS) {
        return 2;
    }

    for (int i = 0; i < NUMBER_OF_MODULES; i++) {
        currentLEDS[i] = (currentLEDS[i] & ~colorsDefinition[kolor]);
    }

    this->updateState();
    return 0;
}

void CPinsController::test() {
    this->stopEasterEgg();

    digitalWrite(PIN_DATA, HIGH);
    for (int i = 0; i < (NUMBER_OF_MODULES * 8); i++)  {
        digitalWrite(PIN_SHIFT, HIGH);
        usleep(WAIT_TIME);
        digitalWrite(PIN_SHIFT, LOW);
    }
    digitalWrite(PIN_DATA, LOW);
    digitalWrite(PIN_LATCH, HIGH);
    usleep(WAIT_TIME);
    digitalWrite(PIN_LATCH, LOW);
    this->updateState();
}

void CPinsController::police() {
    if (easterEggThread.joinable()) {
        easterEggRunning = false;
        easterEggThread.join();
        this->updateState();
        if (currentEasterEgg == POLICE) {
            currentEasterEgg = NONE;
            return;
        }
    }

    easterEggRunning = true;
    easterEggThread = std::thread([this](){
        currentEasterEgg = POLICE;
        unsigned char savedStateOfLEDS[NUMBER_OF_MODULES]{0};
        for (int c = 0; c < NUMBER_OF_MODULES; c++) {
            savedStateOfLEDS[c] = currentLEDS[c];
        }

        while (easterEggRunning) {
            for (int j = 0; j < NUMBER_OF_MODULES; j++) {
                currentLEDS[j] = RED;
            }
            this->updateState();

            for (int j = 0; j < NUMBER_OF_MODULES; j++) {
                currentLEDS[j] = BLUE;
            }
            this->updateState();
        }

        for (int c = 0; c < NUMBER_OF_MODULES; c++) {
            currentLEDS[c] = savedStateOfLEDS[c];
        }
    });
}

void CPinsController::randomize() {
    for (int i = NUMBER_OF_MODULES-1; i >=0; i--) {
        std::bitset<8> bit((char)rand());

        for (int j = 7; j >= 0; j--) {
            digitalWrite(PIN_DATA, bit[j]);
            digitalWrite(PIN_SHIFT, HIGH);
            usleep(WAIT_TIME);
            digitalWrite(PIN_SHIFT, LOW);
        }
    }

    digitalWrite(PIN_LATCH, HIGH);
    usleep(WAIT_TIME);
    digitalWrite(PIN_LATCH, LOW);
}

void CPinsController::welcome() {
    digitalWrite(PIN_DATA, HIGH);

    digitalWrite(PIN_SHIFT, HIGH);
    usleep(WAIT_TIME);
    digitalWrite(PIN_SHIFT, LOW);
    digitalWrite(PIN_LATCH, HIGH);
    usleep(WAIT_TIME);
    digitalWrite(PIN_LATCH, LOW);

    digitalWrite(PIN_DATA, LOW);

    for (int i = 0; i < (NUMBER_OF_MODULES * 8); i++)  {
        delay(50);
        digitalWrite(PIN_SHIFT, HIGH);
        usleep(WAIT_TIME);
        digitalWrite(PIN_SHIFT, LOW);
        digitalWrite(PIN_LATCH, HIGH);
        usleep(WAIT_TIME);
        digitalWrite(PIN_LATCH, LOW);
    }
}

void CPinsController::xmas() {
    if (easterEggThread.joinable()) {
        easterEggRunning = false;
        easterEggThread.join();
        this->updateState();
        if (currentEasterEgg == XMAS) {
            currentEasterEgg = NONE;
            return;
        }
    }

    easterEggRunning = true;
    easterEggThread = std::thread([this](){
        currentEasterEgg = XMAS;
        while (easterEggRunning) {
            this->randomize();
            usleep(WAIT_TIME);
        }
    });
}

std::string CPinsController::dump() {
    std::string toRet {""};

    for (int i = NUMBER_OF_MODULES-1; i >=0; i--) {
        std::bitset<8> x(currentLEDS[i]);

        toRet += x.to_string() + " ";
    }

    return toRet;
}

void CPinsController::updateState() {
    digitalWrite(PIN_RESET, LOW);
    usleep(WAIT_TIME);
    digitalWrite(PIN_RESET, HIGH);

    for (int i = NUMBER_OF_MODULES - 1; i >= 0; i--) {
        std::bitset<8> bit(currentLEDS[i]);

        for (int j = 7; j >= 0; j--) {
            usleep(WAIT_TIME);
            digitalWrite(PIN_DATA, (bit[j] == 1) ? HIGH : LOW);
            usleep(WAIT_TIME);
            digitalWrite(PIN_SHIFT, HIGH);
            usleep(WAIT_TIME);
            digitalWrite(PIN_SHIFT, LOW);
            usleep(WAIT_TIME);
            digitalWrite(PIN_LATCH, HIGH);
            usleep(WAIT_TIME);
            digitalWrite(PIN_LATCH, LOW);
        }

        usleep(10 * WAIT_TIME);
    }

    usleep(WAIT_TIME);
    digitalWrite(PIN_LATCH, HIGH);
    usleep(WAIT_TIME);
    digitalWrite(PIN_LATCH, LOW);
}

void CPinsController::stopEasterEgg() {
    if (easterEggThread.joinable()) {
        easterEggThread.join();
        easterEggRunning = false;
        currentEasterEgg = NONE;
    }
}
