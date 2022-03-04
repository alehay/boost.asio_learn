#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <boost/json.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "./HttpsServer.hpp"

int main(int argc, char* argv[]) 
{
    ConfigServer config;
    config.serverPort = "65500";
    config.currentServerCertificate = "./server01.crt";
    config.currentServerKey = "./server01.key";
    config.diffieHellman = "./dh2048.pem";


    boost::asio::io_context context{1};
    std::make_shared<HttpsServer>( config, "0.0.0.0", context)->Run();
    context.run();

    return 0;
}