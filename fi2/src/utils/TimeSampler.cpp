#include <vector>
#include <chrono>
#include <assert.h>
#include <unordered_map>
#include <algorithm>
#include "TimeSampler.h"
#include "debug.h"

/*static*/
std::unordered_map<std::string, float> TimeSampler::timings;

void TimeSampler::End(const bool print) {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    float time = (float)(duration.count() / 1000.0);
    if (print) {
        outputAlways("Time taken: {:12.4f} ms - {}\n", time, name.c_str());
    }
    done = true;
    timings[name] = time;
}

TimeSampler::~TimeSampler() {
    if (!done) {
        End(autoPrint);
    }
}
