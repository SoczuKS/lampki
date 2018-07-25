#ifndef _PINSCONTROLLER_H
#define _PINSCONTROLLER_H

#include <wiringPi.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <map>
#include <vector>
#include <bitset>
#include <thread>

#include "Structs.h"
#include "Logger.h"
#include "Settings.h"

class CPinsController {
public:
    CPinsController();
    std::string generateStateJSON();
    void show();
    int setLED(int sektor, int kolor, int stan);
    void reset();
    int resetSingleColor(int kolor);
    std::string dump();
    void updateState();
    void test();
    void police();
    void welcome();
    void randomize();
    void xmas();
    void stopEasterEgg();

private:
    unsigned char currentLEDS[NUMBER_OF_MODULES] {0};
    std::thread easterEggThread;
    bool easterEggRunning = false;
    enum {NONE, XMAS, POLICE} currentEasterEgg{NONE};

    std::map<int, int> colorsDefinition = {{0, RED}, {1, BLUE}, {2, YELLOW}, {3, GREEN}, {4, WHITE}};   // zmapowanie przychodzących indeksów koloru do wartości

};
#endif
