#include <chrono>
#include <iostream>
#include <thread>

#include "HttpsServer.hpp"



int main(int argc, const char** argv)
{
  
    Config config;
    config.port = "65500";
    boost::asio::io_context context{ 1 };
    std::make_shared<HttpsServer>(config,  "0.0.0.0", context)->Run();
    context.run();
}