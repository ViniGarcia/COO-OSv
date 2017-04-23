//Library for getting Metrics from userlevel Click
#ifndef CLICKMETRICS
#define CLICKMETRICS

#include <iostream>
#include <fstream>
#include <string>
#include <regex>

class ClickMetrics {
    private:
    std::regex *checkSequence[7];

    public:
    long getCPU();
    long getDisk();
    long getMemory();
    long getNetTX();
    long getNetRX();

};

#endif
