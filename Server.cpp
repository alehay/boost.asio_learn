#include <boost/asio.hpp>
#include <memory>
#include <algorithm>
#include <iostream>

class server
{
private:
    struct session : public std::enable_shared_from_this<session>
    {
    public:
        session(boost::asio::ip::tcp::socket&& sock)
            : sock(std::move(sock))
        {
            
        }
        ~session()
        {
            sock.close();
        }
        void run()
        {
            read();
        }
        void read()
        {
            buff.resize(1024, 0);
            sock.async_read_some(boost::asio::buffer(buff.data(), buff.size()), 
                std::bind(&session::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
        void on_read(boost::system::error_code error, std::size_t bytes)
        {
            if(!error)
            {
                std::cout << std::string(buff.c_str(), bytes) << std::endl;
                write(bytes);
            }
            else
            {
                shutdown("On read: " + error.message());
            }
        }
        void write(std::size_t bytes)
        {
            buff = "Shut up, " + std::string(buff.begin(), buff.begin() + bytes);
            buff.resize(1024);
            sock.async_write_some(boost::asio::buffer(buff.data(), 1024), 
                std::bind(&session::on_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
        void on_write(boost::system::error_code error, std::size_t bytes)
        {
            if(!error)
            {
                read();
            }
            else
            {
                shutdown("On write" + error.message());
            }
        }
        void shutdown(const std::string& what)
        {
            std::cout << what << std::endl;
            sock.close();
        }
    private:
        boost::asio::ip::tcp::socket sock;
        std::string buff;
    };
public:
    server(boost::asio::io_context& context)
        : context(context)
        , acc(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 55001))
    {

    }
    ~server()
    {
        context.run();
    }
    void accept()
    {
        boost::system::error_code error;
        acc.async_accept(std::bind(&server::on_accept, this, std::placeholders::_1, std::placeholders::_2));
    }
private:
    void on_accept(boost::system::error_code error, boost::asio::ip::tcp::socket sock)
    {
        if(!error)
        {
            auto s = std::make_shared<session>(std::move(sock));
            s->run();
        }
        else
        {
            shutdown("On accept: " + error.message());
        }
        accept();
    }
    void shutdown(const std::string& what)
    {
        std::cout << what << std::endl;
        acc.close();
        exit(3);
    }
private:
    boost::asio::io_context& context;
    boost::asio::ip::tcp::acceptor acc;
};

int main()
{
    boost::asio::io_context ctx;
    server serv(ctx);
    serv.accept();
}