#pragma once
#include <array>
#include <span>

template <typename T, std::size_t TSize>
class StableVector {
public:
    StableVector() : vSize(0) {}


    // Iterator class
    class Iterator {
    public:
        explicit Iterator(T* ptr) : ptr_(ptr) {}

        T& operator*() const { return *ptr_; }
        T* operator->() { return ptr_; }

        // Prefix increment
        Iterator& operator++() { ptr_++; return *this; }

        // Comparison operators
        friend bool operator==(const Iterator& a, const Iterator& b) { return a.ptr_ == b.ptr_; }
        friend bool operator!=(const Iterator& a, const Iterator& b) { return a.ptr_ != b.ptr_; }

    private:
        T* ptr_;
    };

    // Const Iterator class
    class ConstIterator {
    public:
        explicit ConstIterator(const T* ptr) : ptr_(ptr) {}

        const T& operator*() const { return *ptr_; }
        const T* operator->() const { return ptr_; }

        // Prefix increment
        ConstIterator& operator++() { ptr_++; return *this; }

        // Comparison operators
        friend bool operator==(const ConstIterator& a, const ConstIterator& b) { return a.ptr_ == b.ptr_; }
        friend bool operator!=(const ConstIterator& a, const ConstIterator& b) { return a.ptr_ != b.ptr_; }

    private:
        const T* ptr_;
    };

    // Iterator methods
    Iterator begin() { return Iterator(&data_[0]); }
    Iterator end() { return Iterator(&data_[vSize]); }

    ConstIterator begin() const { return ConstIterator(&data_[0]); }
    ConstIterator end() const { return ConstIterator(&data_[vSize]); }

    template <typename... Args>
    size_t emplace_back(Args&&... args) {
        if (vSize < TSize) {
            new (&data_[vSize++]) T(std::forward<Args>(args)...);
            return vSize - 1;
        } else {
            check(0, "Run out of slots: maxEntities = {}\n", TSize);
        }
        return (size_t)-1;
    }

    T& back() {
        check(vSize > 0, "Empty vectpr");
        return data_[vSize - 1];
    }

    // Access elements
    const T& operator[](std::size_t index) const {
        return data_[index];
    }

    std::size_t GetSize() const {
        return vSize;
    }

    void SetSize(size_t s) {
        vSize = s;
    }
 
    size_t size() {
        return vSize;
    }

    void Pop(size_t n) {
        vSize -= n;
        check(vSize < TSize && vSize>=0,"popped too far?");
    }

    std::span<T> GetArray() {
        return std::span<T>(data_.data(), vSize);
    }

    std::span<T> GetArrayLast(size_t n) {
        return std::span<T>(data_.data() + (vSize - n), n);
    }
    std::span<T> GetArrayFrom(size_t n) {
        return std::span<T>(data_.data() + n, vSize - n);
    }

private:
    std::array<T, TSize> data_;
    std::size_t vSize = {};
};

