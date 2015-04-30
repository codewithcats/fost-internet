#include <iostream>
#include <thread>


int main() {
    std::cout << "Simple test" << std::endl;

    std::thread server([]() {
    });

    std::thread client([]() {
    });

    server.join();
    client.join();
    return 0;
}

