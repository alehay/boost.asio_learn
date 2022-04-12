#ifndef HTTPS_SERVER_HPP
#define HTTPS_SERVER_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/json.hpp>
#include <functional>
#include <utility>
#include <memory>
#include <string_view>
#include <thread>
#include <algorithm>
#include <filesystem>
#include <variant>
#include <iostream>



struct Config {
    std::string port;
};

class HttpsServer : public std::enable_shared_from_this<HttpsServer>
{
    
public:
    /**
     * @brief Создает объект Https сервера,
     * @param config  структура с настройками.
     * @param fileDir Полный путь к начальной директории файлов
     * @param InetIp Ip, на котором будет HttpServer
     * @param context Объект управления io.
     * @details Передается структура c настроками, в тч для RequestHandler
     */
    HttpsServer(const Config& config, 
                const std::string InetIp,
                 boost::asio::io_context& context);
    ~HttpsServer();
    /**
     * @brief Запускает сервер.
     */
    void Run();
private:
    /**
     * @brief Начинает прием клиентов.
     */
    void DoAccept();
    /**
     * @brief Обрабатывает подключенного клиента.
     * CallBack.
     * @param error Объект проверки ошибки.
     * @param sock tcp-сокет клиента.
     */
    void OnAccept(boost::beast::error_code error, boost::asio::ip::tcp::socket sock);

    void LoadServerCertificate();
 

private:
    unsigned port = 9988;
    Config config;
    std::string fileDir;

    boost::asio::io_context& context;
    boost::asio::ssl::context ctx;
    boost::asio::ip::tcp::acceptor acc;

public:
friend class HttpsSession;
};




/**
 * @brief Класс для обработки одного клиента.
 */
class HttpsSession : public std::enable_shared_from_this<HttpsSession>
{
private:
    /**
     * @brief Шаблонный класс для отправки сообщения в stream
     */
    class Executable
    {
    public:
        /**
         * @brief Конструктор класса Executable
         * @param rf Ссылка на сессию, которой принадлежит объект класса Executable.
         */
        Executable(HttpsSession& rf);
        /**
         * @brief Оператор для отправки ответа клиенту.
         * @param msg response, отправляемвый клиенту.
         */
        template<bool isRequest, class Body, class Fields>
        void operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg)const
        {
            auto sp = std::make_shared<boost::beast::http::message<isRequest, Body, Fields>>(std::move(msg));
            self.mg = sp;

            // Write the response
            boost::beast::http::async_write(
                self.stream,
                *sp,
                boost::beast::bind_front_handler(
                    &HttpsSession::OnWrite,
                    self.shared_from_this(),
                    sp->need_eof()));
        }
    private:
        HttpsSession& self;
    };
public:
    /**
     * @brief Конструктор класса HttpsSession
     * @param socket r-value сокета, через который будем общаться.
     * @param filePath Путь к директории, где лежат медиафайлы.
     * @param context ssl-context для зашифрованного обмена данными.
     * @param host Указатель на HttpsServer. Необходим чтобы сделать запрос в бд.
     */
   // explicit HttpsSession(boost::asio::ip::tcp::socket&& socket, const std::string& filePath, boost::asio::ssl::context& context, std::weak_ptr<HttpsServer> host);
    explicit HttpsSession(boost::asio::ip::tcp::socket&& socket, const std::string& filePath, boost::asio::ssl::context& context);


    /**
     * @brief Запустить обработку клиента.
     */
    void Run();
private:


    boost::json::value ProcessRequest(boost::json::value task);

    /**
     * @brief Метод для получения типа контента, исходя из расширения файла.
     * @param target Путь к файлу, который запрашивается.
     * @return Строку типа контента.
     */
    std::string GetContentType(const std::string& target);
    /**
     * @brief Метод для проверки успешности ssl-рукопожатия
     * @param error Объект хранения ошибки.
     */
    void OnPerformingSsl(boost::system::error_code error);
    /**
     * @brief Метод для обработки запроса от пользователя.
     * @param docRoot Директория, где лежат файлы.
     * @param req Сообщение-запрос от клиента.
     * @param send Объект, с перегруженным оператором operator().
     */
    template<class Body, class Allocator, class Send>
    void HandleRequest(std::string docRoot, boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req, Send&& send)
    {
        #ifdef DEBUG_PRINT//   
            std::cout << " ?80 HandleRequest RUN" << std::endl;  
            std::cout << "->" << req.body() << "<-" << std::endl;
        #endif



        auto const HandleGet =
        [&req, &docRoot, this]()
        {
            std::string body_str(req.target());
            auto index = body_str.find(this->filePath);
            std::string path(body_str.begin() + index, body_str.end());
            boost::beast::error_code ec;
            boost::beast::http::file_body::value_type body;
            body.open(path.c_str(), boost::beast::file_mode::scan, ec);

            auto const size = body.size();

            boost::beast::http::response<boost::beast::http::file_body> res{
                std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(boost::beast::http::status::ok, req.version())};
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(boost::beast::http::field::content_type, GetContentType(path));
            res.content_length(size);
            res.keep_alive(false);

            return res;
        };



        if( req.method() != boost::beast::http::verb::get )
        {
            // spdlog::warn("Unknown HTTP-method");
            send(BadRequest("Unknown HTTP-method <-"));
            return;
        }

        if(req.method() == boost::beast::http::verb::get)
        {

            std::cout << "Get method " << std::endl;
            send(HandleGet());
            return;
        }


    }
    /**
     * @brief Метод, запускаемый после Run()
     */
    void OnRun();
    /**
     * @brief Метод для чтения запроса клиента.
     */
    void DoRead();
    /**
     * @brief Метод для проверки успешности чтения от клиента.
     * @param error Объект для хранения ошибки.
     * @param bytes_transferred Сколько байт пришло.
     */
    void OnRead(boost::system::error_code error, std::size_t bytes_transferred);
    /**
     * @brief Метод для проверки успешности отсылки ответа клиенту.
     * @param close Отключаем ли клиента.
     * @param ec Объект для хранения ошибки.
     * @param bytes_transferred Сколько байт ушло.
     */
    void OnWrite(bool close, boost::beast::error_code ec, std::size_t bytes_transferred);
    /**
     * @brief Метод для отключения клиента.
     */
    void DoClose();
    /**
     * @brief Метод для проверки того, отключился ли клиент.
     * @param error Объект для хранения ошибки.
     */
    void OnShutdown(boost::beast::error_code error);
private:
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream;
    boost::beast::flat_buffer buff;
    boost::beast::http::request<boost::beast::http::string_body> req;
    std::shared_ptr<void> mg;
    Executable exec;
     // std::weak_ptr<HttpsServer> host;
    std::string filePath;
};

#endif//HTTPS_SERVER_HPP