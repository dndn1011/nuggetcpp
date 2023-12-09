#include <iostream>

struct Dimension {
    operator float() const {
        return f;
    }
    // Example member variable 'f'
    float f;
};

int main() {
    // Create an object of the class or struct that contains the provided operator()
    Dimension obj;

    // Set the value of 'f' (assuming 'f' is a public member variable or there is a setter function)
    obj.f = 3.14f;

    // Use the operator() to "call" the object like a function and pass a Dimension object
    float result = obj;

    // Print the result
    std::cout << "Result: " << result << std::endl;

    return 0;
}