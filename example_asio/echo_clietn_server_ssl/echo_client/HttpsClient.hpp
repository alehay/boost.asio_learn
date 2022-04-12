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
    HttpsClient(boost::asio::io_context& context, const Config& config);
    /**
     * @brief Отправляет Post-запрос на взятие параметров файла. В теле
     * {
     *      "date_from" : "2000-01-01 01:01:01",
     *      "date_to" : "2022-01-01 01:01:01",
     *      "project_id" : ["10", "11"],
     *      "theme" : ["123", "321", "21525"],
     *      "agent_id" : ["52200", "125525"]
     * }
     * @param task json, содержащий тело запроса(URL файла, который грузим)
     * @param bad ссылка на bool, true, если во время работы произошла ошибка
     * @return boost::json::value URL, где находится json::array, в котором все параметры файлов, кроме путей к хранилищам
     * {
     *      "files" : [
     *          "created" : ...,
     *          "pin" : ...,
     *          ...
     *      ]
     * }
     */
    std::string PostRequest(const std::string& task, bool& bad);
    std::string GetRequest();

private:
    //Для работы с https
    boost::asio::ssl::context ssl_context;
    //Для работы с IO
    boost::asio::io_context& context;

    //Для перевода host-name в ip
    boost::asio::ip::tcp::resolver resolver;
    //Точка подключения к серверу
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> results;

    Config config;
};

#endif//SOURCE_HTTPS_CLIENT