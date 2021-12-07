#include <boost/asio.hpp>
#include <algorithm>
#include <iostream>

class client
{
public:
    client(boost::asio::io_context& context)
        : context(context)
        , sock(context)
    {
        buff.resize(1024, 0);
    }
    void connect(const std::string& ip, short port)
    {
        sock.async_connect(
            boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(ip), port),
            std::bind(&client::on_connect, this, std::placeholders::_1));
    }
    ~client()
    {
        context.run();
    }
private:
    void on_connect(boost::system::error_code error)
    {
        if(!error)
        {
            write();
        }
        else
        {
            shutdown("On connect: " + error.message());
        }
    }
    void write()
    {
        std::cout << "Write message: ";
        std::cin >> buff;
        buff.resize(1024);
        sock.async_write_some(boost::asio::buffer(buff.data(), 1024), 
            std::bind(&client::on_write, this, std::placeholders::_1, std::placeholders::_2));
    }
    void on_write(boost::system::error_code error, std::size_t bytes)
    {
        if(!error)
        {
            read();
        }
        else
        {
            shutdown("On write: " + error.message());
        }
    }
    void read()
    {
        buff.resize(1024, 0);
        sock.async_read_some(boost::asio::buffer(buff.data(), 1024), 
            std::bind(&client::on_read, this, std::placeholders::_1, std::placeholders::_2));
    }
    void on_read(boost::system::error_code error, std::size_t bytes)
    {
        if(!error)
        {
            std::cout << "From server: " << buff << std::endl;
            write();
        }
        else
        {
            shutdown("On read: " + error.message());
        }
    }
    void shutdown(const std::string& what)
    {
        std::cout << what << std::endl;
        sock.close();
        exit(3);
    }
private:
    boost::asio::io_context& context;
    boost::asio::ip::tcp::socket sock;
    std::string buff;
};

int main()
{
    boost::asio::io_context ctx;

    client cl(ctx);

    cl.connect("127.0.0.1", 55001);
}