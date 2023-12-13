#pragma once
#include <array>
#include <span>

template <typename T, std::size_t TSize>
class StableVector {
public:
    StableVector() : size(0) {}

    template <typename... Args>
    size_t emplace_back(Args&&... args) {
        if (size < TSize) {
            new (&data_[size++]) T(std::forward<Args>(args)...);
            return size - 1;
        } else {
            check(0, "Run out of slots: maxEntities = {}\n", TSize);
        }
        return (size_t)-1;
    }

    // Access elements
    const T& operator[](std::size_t index) const {
        return data_[index];
    }

    std::size_t GetSize() const {
        return size;
    }

    std::span<T> GetArray() {
        return std::span<T>(data_.data(), size);
    }

private:
    std::array<T, TSize> data_;
    std::size_t size = {};
};

