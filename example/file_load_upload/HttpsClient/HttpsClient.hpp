#ifndef SOURCE_HTTPS_CLIENT
#define SOURCE_HTTPS_CLIENT

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

class HttpsClient {
 public:
  /**
   * @brief Конструктор класса HttpsClient
   * @param context Объект для io-операций
   * @param config Конфиг программы
   */
  HttpsClient(boost::asio::io_context& context);
  /**
   * @brief Отправляет Post-запрос
   * @param task json, содержащий тело запроса(URL файла, который грузим)
   * @param bad ссылка на bool, true, если во время работы произошла ошибка

   */
  std::string PostRequest(const std::string& task, bool& bad);
  /**
   * @brief file upload request
   *
   * @param fileData информация о файле на локальном устройстве и на сервере
   * @param bad false, если прием прошел нормально
   * @param
   */
  std::string GetRequest(const std::string& filePath,
                         const std::string& saveFilePath, bool& bad);

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

#endif  // SOURCE_HTTPS_CLIENT