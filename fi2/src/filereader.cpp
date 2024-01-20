#include "filereader.h"
#include "debug.h"

#include <thread>
#include <chrono>

namespace nugget {
    FileReader::FileReader(const std::string& filename) : filename(filename), buffer(chunkSize, '\0'), lastReadPosition(std::streampos(0)) {}

    // Read up to 512 bytes from the specified position in the file
    std::string FileReader::readBytesFromPosition(std::streampos position) {

        check(file.is_open(),"file closed?");

        std::streampos newPosition = position;

        if (newPosition >= lastReadPosition && newPosition + static_cast<std::streampos>(chunkSize) < lastReadPosition + static_cast<std::streampos>(bufferLength)) {
            std::size_t offset = static_cast<std::size_t>(newPosition - lastReadPosition);

            // Return the substring from the buffer
            return std::string(buffer.begin() + offset, buffer.begin() + offset + chunkSize);

        } else if (newPosition >= lastReadPosition + static_cast<std::streampos>(buffer.size())) {
            file.read(buffer.data(), buffer.size());
            bufferLength = file.gcount();
            return std::string(buffer.begin(), buffer.begin() + chunkSize);
        } else {
            // Move existing data to the start of the buffer
            size_t offset = newPosition - lastReadPosition;
            std::copy(buffer.begin() + offset, buffer.end(), buffer.begin());
            bufferLength -= offset;

            // Read additional bytes to fill up the buffer
            if (file.seekg(lastReadPosition + static_cast<std::streampos>(bufferLength + offset))) {
                file.read(buffer.data() + bufferLength, chunkSize - bufferLength);
                bufferLength += file.gcount();
            }

            lastReadPosition = newPosition;

            return std::string(buffer.begin(), buffer.begin() + bufferLength);
        }
    }

    void FileReader::Open() {
        int count;
        for (
            count = 0, file = std::ifstream(filename, std::ios::binary);
            !file.is_open() && count < 10;
            std::this_thread::sleep_for(std::chrono::milliseconds(100)), count++
            );

        check(file.is_open(), "Could not open file: {}\n", filename);
    }

    FileReader::~FileReader() {
        file.close();
    }
}