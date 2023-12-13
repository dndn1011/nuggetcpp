#pragma once
#include <chrono>
#include <assert.h>
#include <unordered_map>

#include "utils.h"

// Handy class to measure durations
class TimeSampler {
    static std::unordered_map<std::string, float> timings;
    std::chrono::steady_clock::time_point start;
    std::string name;
    bool done;
    bool autoPrint;
public:

    APPLY_RULE_OF_MINUS_5(TimeSampler);

    TimeSampler(const std::string& name,bool autoPrint = false) : name(name), autoPrint(autoPrint),start(std::chrono::high_resolution_clock::now()), done(false) {
    }

    static float GetTiming(const std::string& name) {
        return timings[name];
    }

    void End(const bool print = true);

    ~TimeSampler();
};
