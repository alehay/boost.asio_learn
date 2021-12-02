#include <boost/asio.hpp>
#include <iostream>
using namespace boost;

std::string readFromSocket(asio::ip::tcp::socket& sock) {
    const unsigned char MESSAGE_SIZE = 18;
    char buf[MESSAGE_SIZE];
    std::size_t total_bytes_read = 0;
    while (total_bytes_read != MESSAGE_SIZE) {
        
        total_bytes_read += sock.read_some(
        
        asio::buffer(buf + total_bytes_read,
        MESSAGE_SIZE - total_bytes_read));
    }
    return std::string(buf, total_bytes_read);
}


int main()
{
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 8081;
    
    try {
        asio::ip::tcp::endpoint
        ep(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_service ios;
        asio::ip::tcp::socket sock(ios, ep.protocol());
        sock.connect(ep);
        
        std::string message = readFromSocket(sock);
        std::cout << message << std::endl; 
    }
    catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code() << ". Message: " << e.what();
        return e.code().value();
    }
    return 0;
}