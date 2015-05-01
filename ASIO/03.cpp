#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>


int main() {
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    std::condition_variable signal;

    boost::asio::io_service service;

    boost::asio::ip::tcp::acceptor listener(service);
    boost::asio::ip::tcp::socket server_socket(service);
    std::thread server([&]() {
        std::unique_lock<std::mutex> lock(mutex);
        listener.open(boost::asio::ip::tcp::v4());
        listener.set_option(boost::asio::socket_base::enable_connection_aborted(true));
        listener.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 4567));
        listener.listen();
        listener.async_accept(server_socket, [](const boost::system::error_code& error) {
            std::cout << "Got a connection " << error << std::endl;
        });
        signal.notify_one();
        std::cout << "Server set up" << std::endl;
    });
    signal.wait(lock);

    boost::asio::ip::tcp::socket client_socket(service);
    std::thread client([&]() {
        boost::asio::ip::tcp::endpoint address(boost::asio::ip::address_v4(0ul), 4567);
        client_socket.async_connect(address, [](const boost::system::error_code& error) {
            std::cout << "Connected " << error << std::endl;
        });
        signal.notify_one();
        std::cout << "Client set up" << std::endl;
    });
    signal.wait(lock);

    std::thread io([&]() {
        std::cout << "About to service IO requests" << std::endl;
        service.run();
        std::cout << "Service jobs all run" << std::endl;
        signal.notify_one();
    });
    signal.wait(lock);

    io.join();
    client.join();
    server.join();
    return 0;
}

