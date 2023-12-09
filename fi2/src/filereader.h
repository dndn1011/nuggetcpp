#pragma once
#include <iostream>
#include <fstream>
#include <vector>

namespace nugget {
    class FileReader {
        const int chunkSize = 512;
    public:
        FileReader(const std::string& filename);

        // Read up to 512 bytes from the specified position in the file
        std::string readBytesFromPosition(std::streampos position);

        void Open();

        ~FileReader();

    private:
        std::ifstream file;
        std::string filename;
        std::vector<char> buffer;
        size_t bufferLength = 0;
        std::streampos lastReadPosition;
    };
}