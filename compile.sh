g++ -std=c++14 -Wall -pedantic -lwiringPi -pthread src/main.cpp -o lampki.bin -Os -s
COMPILE_RESULT=$?
ls -laF --color lampki.bin
exit $COMPILE_RESULT
