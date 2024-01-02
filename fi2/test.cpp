#include <iostream>
#include <map>
#include <utility>

class NonCopyableNonMovable {
public:
    NonCopyableNonMovable() = default;

    NonCopyableNonMovable(const NonCopyableNonMovable&) = delete;
    NonCopyableNonMovable& operator=(const NonCopyableNonMovable&) = delete;

    NonCopyableNonMovable(NonCopyableNonMovable&&) = delete;
    NonCopyableNonMovable& operator=(NonCopyableNonMovable&&) = delete;

    void print() const {
        std::cout << "NonCopyableNonMovable object" << std::endl;
    }
};

int main() {
    std::map<int, NonCopyableNonMovable> myMap;

    // Use emplace with std::piecewise_construct
    myMap.emplace(std::piecewise_construct, std::forward_as_tuple(1), std::forward_as_tuple());

    // Access the element and call a member function
    myMap[1].print();

    // Uncommenting the lines below would result in compilation errors
    // NonCopyableNonMovable obj = myMap[1];  // Error: copy constructor is deleted
    // NonCopyableNonMovable obj2(std::move(myMap[1]));  // Error: move constructor is deleted

    return 0;
}