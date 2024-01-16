#include <string>
#include <cmath>
#include <iostream>
#include <random>
#include <chrono>
#include <assert.h>

void floatToString(char *result,float value, int precision = 6) {
    char integerPartResult[256];
    char* resultp = result;
    char* integerPartResultp = integerPartResult;

    // Handle negative numbers
    if (value < 0) {
        *(resultp++) = '-';
        value = -value;
    }

    // Extract integer part
    int integerPart = (int)(value);
    float fractionalPart = value - (float)(integerPart);
    assert(value == (value - (float)integerPart + (float)integerPart));

    // Convert integer part to string
    do {
        int div10 = integerPart / 10;
        char digit = '0' + (char)((integerPart - 10*div10));
        *(integerPartResultp++) = digit;
        integerPart = div10;
    } while (integerPart != 0);

    for (char* p = integerPartResultp - 1; p >= integerPartResult; p--) {
        *(resultp++) = *p;
    }

    *(resultp++) = '.';

    char digit;
    // Convert fractional part to string^
    while (precision-- > 0) {
        fractionalPart *= 10;
        digit = '0' + (int)(fractionalPart) % 10;
        *(resultp++) = digit;
        fractionalPart -= (int)(fractionalPart);
    }
    *(resultp++) = '\0';
}

int main() {
    // Set up random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1000000, 1000000);

    // Generate and convert 1 million random floats
    const int numTests = 1000000;
    std::vector<float> randomFloats(numTests);
    std::vector<std::string> stringResults(numTests);
    std::vector<std::string> stringResults2(numTests);

    for (int i = 0; i < numTests; ++i) {
        randomFloats[i] = (float)dis(gen);
    }

    {
        // Start timer
        auto start = std::chrono::high_resolution_clock::now();


        for (int i = 0; i < numTests; ++i) {
            char r[256];
            floatToString(r,randomFloats[i], 6);
            stringResults[i] = r;
        }

        // End timer
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        // Output time taken
        std::cout << "Algo: " << elapsed.count() << " seconds." << std::endl;
    }

    {
        // Start timer
        auto start = std::chrono::high_resolution_clock::now();


        for (int i = 0; i < numTests; ++i) {
            stringResults2[i] = std::to_string(randomFloats[i]);
        }

        // End timer
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        // Output time taken
        std::cout << "std:  " << elapsed.count() << " seconds." << std::endl;
    }

    for (int i = 0; i < 10; i++) {
        std::printf("test input: %f\n", randomFloats[i]);
    }

    for (int i = 0; i < numTests; ++i) {
//        printf(" ?  %d %f %s %s\n", i, (double)randomFloats[i], stringResults[i].c_str(), stringResults2[i].c_str());
        float a;
        float b;
        
        std::from_chars(stringResults[i].data(), stringResults[i].data() + stringResults[i].size(), a);
        std::from_chars(stringResults2[i].data(), stringResults2[i].data() + stringResults2[i].size(), b);

        if (abs(a - b) > 0.000002f) {
            printf("oh no! %d %f %s %s\n", i, (double)randomFloats[i], stringResults[i].c_str(), stringResults2[i].c_str());
            break;
        }
    }

//    std::string myStr = std::to_string(myFloat);
    std::cout << "DONE!" << std::endl;
    getchar();
    return 0;
}

