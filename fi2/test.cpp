#include <iostream>
#include <array>

#define APPLY_RULE_OF_MINUS_5(Imp) \
    Imp() = delete;\
    Imp(const Imp& other) = delete;\
    Imp(Imp&& other) = delete;\
    Imp& operator=(const Imp& other) = delete;\
    Imp& operator=(Imp&& other) = delete

#define APPLY_RULE_OF_MINUS_4(Imp) \
    Imp(const Imp& other) = delete;\
    Imp(Imp&& other) = delete;\
    Imp& operator=(const Imp& other) = delete;\
    Imp& operator=(Imp&& other) = delete

struct Thing {
    APPLY_RULE_OF_MINUS_4(Thing);

    template <typename T, std::size_t Size>
    friend class StableVector;  // Declare StableVector as a friend

    int GetValue() const {
        return haha;
    }

    Thing(int val) : haha(val) {}
    Thing() : haha(0) {}

protected:
    ~Thing() = default;  // Make the destructor private
private:
    int haha;
};

template <typename T, std::size_t Size>
class StableVector {
public:
    // Constructor initializes the buffer size
    StableVector() : size(0) {}

    // Emplace_back adds an element to the buffer
    template <typename... Args>
    void emplace_back(Args&&... args) {
        if (size < Size) {
            new (&data_[size++]) T(std::forward<Args>(args)...);
        } else {
            // Handle buffer full (you might throw an exception or resize the buffer)
            // Here, we just do nothing when the buffer is full.
        }
    }

    // Access elements
    const T& operator[](std::size_t index) const {
        return data_[index];
    }

    std::size_t GetSize() const {
        return size;
    }

private:
    std::array<T, Size> data_;
    std::size_t size = {};
};

int main() {
    // Usage example
    StableVector<Thing, 5> stableVector;

    stableVector.emplace_back(1);
    stableVector.emplace_back(2);
    stableVector.emplace_back(3);

    for (std::size_t i = 0; i < stableVector.size(); ++i) {
        std::cout << stableVector[i].GetValue() << " ";
    }

    getchar();
    return 0;
}