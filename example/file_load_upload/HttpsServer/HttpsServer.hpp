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


class HttpsServer : public std::enable_shared_from_this<HttpsServer>
{
    friend class HttpsSession;
public:
    /**
     * @brief Создает объект Https сервера,
     * @param config  структура с настройками.
     * @param fileDir Полный путь к начальной директории файлов
     * @param InetIp Ip, на котором будет HttpServer
     * @param context Объект управления io.
     * @details Передается структура c настроками, в тч для RequestHandler
     */
    HttpsServer(const Config& config, const std::string& fileDir, const std::string InetIp, boost::asio::io_context& context);
    ~HttpsServer();
    /**
     * @brief Запускает сервер.
     */
    void Run();
private:
    /**
     * @brief Метод для получения типа контента, исходя из расширения файла.
     * @param target Путь к файлу, который запрашивается.
     * @return Строку типа контента.
     */
    std::string GetContentType(const std::string& target);
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
    /**
    * @brief Обработка запроса на выдачу файлов по json
    * @param task тело запроса
    * @param bad - false, если с ошибкой.
    * @return результат выполнения запроса в виде json value
    * @details ожидает task в виде json формата


    * создает объект RequestHandler настроенный из , передает ему
    структуру RequestHandler::User
    * заполненную из Json, параметрами, и возвращает результат
    *  SelectFileNames запроса обернутый в валидный json array
    */
    boost::json::value ProcessRequest(boost::json::value task, bool& bad);
    /**
     * @brief Метод для загрузки серверного сертификата, RSA-ключа и DH-параметра.
     */
    void LoadServerCertificate();
    /**
     * @brief Вспомогательный метод для загрузки пароля, с которым был создан сертификат.
     * @return Пароль
     */
    std::string GetPassword()const;

    boost::beast::http::response<boost::beast::http::string_body> 
        Error(boost::beast::http::status status, const std::string& what,unsigned version);
    //Взять все параметры файлов по метаданным
    boost::beast::http::response<boost::beast::http::string_body> 
        HandlePostFindWithMeta(boost::beast::http::request<boost::beast::http::string_body>&& req);
    //Взять URL файлов по параметрам
    std::variant<boost::beast::http::response<boost::beast::http::file_body>,
                boost::beast::http::response<boost::beast::http::string_body>>
        HandleGetLoad(boost::beast::http::request<boost::beast::http::string_body>&& req);
private:
    const unsigned port = 9988;
    Config config;
    std::string fileDir;

    boost::asio::io_context& context;
    boost::asio::ssl::context ctx;
    boost::asio::ip::tcp::acceptor acc;
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
    explicit HttpsSession(boost::asio::ip::tcp::socket&& socket, const std::string& filePath, boost::asio::ssl::context& context, std::weak_ptr<HttpsServer> host);
    /**
     * @brief Запустить обработку клиента.
     */
    void Run();
private:
    /**
     * @brief Метод для проверки успешности ssl-рукопожатия
     * @param error Объект хранения ошибки.
     */
    void OnPerformingSsl(boost::system::error_code error);

    boost::beast::http::response<boost::beast::http::string_body> 
        Error(boost::beast::http::status status, const std::string& what,unsigned version);
    /**
     * @brief Метод для обработки запроса от пользователя.
     * @param req Сообщение-запрос от клиента.
     * @param send Объект, с перегруженным оператором operator().
     */
    void HandleRequest(boost::beast::http::request<boost::beast::http::string_body>&& req, Executable& send);
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
    std::weak_ptr<HttpsServer> host;
    std::string filePath;
};

#endif//HTTPS_SERVER_HPP