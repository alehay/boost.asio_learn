#ifndef SOURCE_HTTPS_CLIENT
#define SOURCE_HTTPS_CLIENT

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/json.hpp>
#include <thread>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>


struct Config {
    std::string rootCACertificate;
    std::string serverHost;
    std::string serverPort;
};

class HttpsClient
{
public:
    /**
     * @brief Конструктор класса HttpsClient
     * @param context Объект для io-операций
     * @param config Конфиг программы
     */
    HttpsClient(boost::asio::io_context& context, Config& config);
    /**
     * Пример: client.PostRequest(tast, "/v1/load", bad);
     * Или client.PostRequest(tast, "/v1/unloading", bad);
     */
    std::string PostRequest(const std::string& task, const std::string& target, bool& bad);
private:
    //Для работы с https
    boost::asio::ssl::context ssl_context;
    //Для работы с IO
    boost::asio::io_context& context;

    //Для перевода host-name в ip
    boost::asio::ip::tcp::resolver resolver;
    //Точка подключения к серверу
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> results;
};

#endif//SOURCE_HTTPS_CLIENT