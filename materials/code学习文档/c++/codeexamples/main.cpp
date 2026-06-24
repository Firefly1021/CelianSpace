#include <memory>
#include <iostream>

void demo() {
    auto ptr = std::make_shared<int>(100);
    std::cout << "Value: " << *ptr << std::endl;
}