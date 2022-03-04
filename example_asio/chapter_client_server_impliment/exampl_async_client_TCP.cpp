#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

using namespace boost;

// Function pointer type that points to the callback
// function which is called when a request is complete.
typedef void (*Callback)(unsigned int request_id, const std::string& response,
                         const system::error_code& ec);

// Structure represents a context of a single request.
struct Session {
  Session(asio::io_service& ios, const std::string& raw_ip_address,
          unsigned short port_num, const std::string& request, unsigned int id,
          Callback callback)
      : m_sock(ios),
        m_ep(asio::ip::address::from_string(raw_ip_address), port_num),
        m_request(request),
        m_id(id),
        m_callback(callback),
        m_was_cancelled(false) {}
  asio::ip::tcp::socket m_sock;  // Socket used for communication
  asio::ip::tcp::endpoint m_ep;  // Remote endpoint.
  std::string m_request;
  // Request string.
  // streambuf where the response will be stored.
  asio::streambuf m_response_buf;
  std::string m_response;  // Response represented as a string.
  // Contains the description of an error if one occurs during
  // the request life cycle.
  system::error_code m_ec;
  unsigned int m_id;  // Unique ID assigned to the request.
  // Pointer to the function to be called when the request
  // completes.
  Callback m_callback;
  bool m_was_cancelled;
  std::mutex m_cancel_guard;
};

class AsyncTCPClient : public boost::noncopyable {
 public:
  AsyncTCPClient() {
    //Первый — поток пользовательского интерфейса — отвечает за
    // обработку пользовательского ввода и инициирование запросов.
    // Ответственность второго потока — потока ввода—вывода - заключается
    // в запуске цикла событий и вызове процедур обратного вызова асинхронной
    // операции.

    //  экземпляр объекта класса asio::io_service::work, передавая экземпляр
    //  класса asio::io_service с именем m_ios своему конструктору.
    m_work.reset(new boost::asio::io_service::work(m_ios));
    // unique_ptr::reset Принимает право на владение параметром указателя,
    // а затем удаляет исходный сохраненный указатель.
    // Если новый указатель совпадает с исходным сохраненным указателем,
    // reset удаляет указатель и задает для сохраненного указателя значение
    // nullptr

    //  создает поток, который вызывает метод run() объекта them_ios. Объект
    //  класса asio::io_service::work не позволяет потокам, выполняющим цикл
    //  событий, выходить из этого цикла, когда нет ожидающих асинхронных
    //  операций. Созданный поток играет роль потока ввода-вывода в нашем
    //  приложении; в контексте этого потока будут вызываться обратные вызовы,
    //  назначенные асинхронным операциям.
    m_thread.reset(new std::thread([this]() { m_ios.run(); }));
  }

  // demonstathion class,
  // initiates a request to the server

  /**
   * @brief  предназначен для инициирования асинхронного запроса
   *
   * @param duration_sec  представляет параметр запроса в соответствии с
   * протоколом прикладного уровня (???)
   * @param raw_ip_address  адрес и номер порта указывают сервер, на который
   * должен быть отправлен запрос.
   * @param port_num
   * @param callback  указатель на функцию обратного вызова, которая будет
   * вызвана после завершения запроса
   * @param request_id это уникальный идентификатор запроса. Этот идентификатор
   * связан с запросом и используется для ссылки на него позже, например, когда
   * возникает необходимость его отменить.
   */
  void emulateLongComputationOp(unsigned int duration_sec,
                                const std::string& raw_ip_address,
                                unsigned short port_num, Callback callback,
                                unsigned int request_id) {
    // Preparing the request string.
    // подготовки строки запроса и
    std::string request =
        "EMULATE_LONG_CALC_OP " + std::to_string(duration_sec) + "\n";

    // выделения экземпляра структуры сеанса, которая хранит данные, связанные с
    // запросом, включая объект сокета, используемый для связи с сервером.
    std::shared_ptr<Session> session = std::shared_ptr<Session>(new Session(
        m_ios, raw_ip_address, port_num, request, request_id, callback));
    // Затем сокет открывается, и указатель на объект сеанса добавляется  в map
    // them_active_sessions.
    session->m_sock.open(session->m_ep.protocol());
    // Add new session to the list of active sessions so
    // that we can access it if the user decides to cancel
    // the corresponding request before it completes.
    // Because active sessions list can be accessed from
    // multiple threads, we guard it with a mutex to avoid
    // data corruption.

    // Синхронизация необходима, поскольку к map m_active_sessions можно
    // получить доступ из нескольких потоков. Элементы добавляются в него в
    // потоке пользовательского интерфейса и удаляются в потоке ввода-вывода,
    // который вызывает обратный вызов после завершения соответствующего
    // запроса.
    std::unique_lock<std::mutex> lock(m_active_sessions_guard);

    // map m_active_sessions содержит указатели на объекты сеанса, связанные
    // со всеми активными запросами, то есть теми запросами, которые были
    // инициированы, но еще не завершены. Когда запрос завершается, перед
    // вызовом соответствующего обратного вызова указатель на объект сеанса,
    // связанный с этим запросом, удаляется  из  map m_active_sessions.
    // Аргумент request_id используется в качестве ключа соответствующего
    // объекта сеанса, добавленного на карту. Нам нужно кэшировать объекты
    // сеанса, чтобы иметь к ним доступ в случае, если пользователь решит
    // отменить ранее инициированный запрос. Если бы нам не нужно было
    // поддерживать отмену запроса, мы могли бы избежать использования карты
    // them_active_sessions.
    m_active_sessions[request_id] = session;

    lock.unlock();

    // Теперь, когда указатель на соответствующий объект сеанса сохранен, нам
    // нужно подключить сокет к серверу, что мы делаем, вызывая метод
    // async_connect() сокета:
    session->m_sock.async_connect(
        // В нашем примере мы используем лямбда-функцию в качестве функции
        // обратного вызова. Вызов метода async_connect() сокета является
        // последним оператором в методе emulateLongComputationOp().
        // Когда возвращается функция async_connect(), также возвращается
        // функция emulateLongComputationOp(), что означает, что запрос
        // был инициирован.
        session->m_ep, [this, session](const system::error_code& ec) {
          // проверки кода ошибки, переданного ему в качестве аргумента ec,
          // значение которого, отличное от нуля, означает, что соответствующая
          // асинхронная операция завершилась неудачно.
          if (ec) {
            // В случае сбоя мы сохраняем значение ec в соответствующем объекте
            // сеанса,
            session->m_ec = ec;
            // вызываем метод onRequestComplete() класса,
            // передавая ему объект сеанса в качестве аргумента
            onRequestComplete(session);
            return;
          }

          // Если ошибки нет
          // блокируем мьютекс them_cancel_guard (член объекта дескриптора
          // запроса)
          std::unique_lock<std::mutex> cancel_lock(session->m_cancel_guard);

          // и проверяем, не был ли запрос еще отменен
          if (session->m_was_cancelled) {
            onRequestComplete(session);
            return;
          }

          // Если мы видим, что запрос не был отменен, мы инициируем следующую
          // асинхронную операцию
          //  async_write() для отправки данных запроса на сервер. Опять же, мы
          //  передаем ему лямбда-функцию в качестве обратного вызова.
          asio::async_write(
              session->m_sock, asio::buffer(session->m_request),
              [this, session](const boost::system::error_code& ec,
                              std::size_t bytes_transferred) {
                // проверяем код ошибки
                if (ec) {
                  session->m_ec = ec;
                  onRequestComplete(session);
                  return;
                }
                //

                std::unique_lock<std::mutex> cancel_lock(
                    session->m_cancel_guard);

                // проверяем был ли отменен запрос
                if (session->m_was_cancelled) {
                  onRequestComplete(session);
                  return;
                }

                // читаем ответ с сервера
                asio::async_read_until(
                    session->m_sock, session->m_response_buf, '\n',
                    // передаем функцию обратного вызова
                    //
                    [this, session](const boost::system::error_code& ec,
                                    std::size_t bytes_transferred) {
                      // проверяем код ошики
                      if (ec) {
                        session->m_ec = ec;
                      } else {
                        //  сохраняет полученные данные ответа в соответствующем
                        //  объекте сеанса.
                        std::istream strm(&session->m_response_buf);
                        std::getline(strm, session->m_response);
                      }
                      // метод класса AsyncTCPClient onrequestcomplete(), и
                      // объект сеанса передается ему в качестве аргумента.
                      onRequestComplete(session);
                    });
              });
        });
  };

  /**
   * @brief  Этот метод принимает идентификатор запроса, подлежащего отмене, в
   * качестве аргумента. Он начинается с поиска объекта сеанса, соответствующего
   * указанному запросу, m_active_sessions
   *
   * @param request_id
   */
  void cancelRequest(unsigned int request_id) {
    std::unique_lock<std::mutex> lock(m_active_sessions_guard);
    // Начинаем искать отмененный сеанс
    auto it = m_active_sessions.find(request_id);
    //  Если таковой найден, он вызывает метод cancel() для объекта сокета,
    //  хранящегося в этом объекте сеанса.
    if (it != m_active_sessions.end()) {
      std::unique_lock<std::mutex> cancel_lock(it->second->m_cancel_guard);
      it->second->m_was_cancelled = true;
      it->second->m_sock.cancel();
    }
  }
  // Однако есть вероятность, что метод cancelRequest() будет вызван в тот
  // момент, когда одна асинхронная операция уже завершена, а следующая еще не
  // начата. Например, представьте, что поток ввода-вывода теперь выполняет
  // обратный вызов операции async_connect(), связанной с определенным сокетом.
  // В данный момент асинхронная операция, связанная с этим сокетом, не
  // выполняется (поскольку следующая асинхронная операция async_write() еще не
  // была инициирована); поэтому вызов cancel() для этого сокета не будет иметь
  // никакого эффекта. Вот почему мы используем дополнительный флаг
  // Session::m_was_cancelled, обозначающий, как следует из его названия, был ли
  // отменен запрос (или, если быть более точным, был ли вызван метод
  // cancelrequest() пользователем).

  void close() {
    // Destroy work object. This allows the I/O thread to
    // exits the event loop when there are no more pending
    // asynchronous operations.

    // . Во-первых, этот метод уничтожает объект them_work, который позволяет потоку ввода-вывода выходить из цикла сообщений о событиях после завершения всех асинхронных операций
    m_work.reset(NULL);
    // Wait for the I/O thread to exit.

    // Затем он присоединяется к потоку ввода-вывода, чтобы дождаться его завершения. 
    m_thread->join();
  }

 private:
  /**
   * @brief  вызывается всякий раз, когда
   * запрос завершается с каким-либо результатом. Он
   * вызывается, когда запрос успешно завершается, когда
   * запрос завершается неудачно на любом этапе его
   * жизненного цикла или когда он отменяется пользователем.
   * Цель этого метода состоит в том, чтобы выполнить
   * очистку, а затем вызвать обратный вызов,
   * предоставляемый вызывающим объектом метода
   *
   * @param session
   */
  void onRequestComplete(std::shared_ptr<Session> session) {
    // Shutting down the connection. This method may
    // fail in case socket is not connected. We don’t care
    // about the error code if this function fails.

    //
    boost::system::error_code ignored_ec;

    // Метод onRequestComplete() начинается с выключения сокета. Обратите
    // внимание, что здесь мы используем перегрузку метода shutdown() сокета,
    // который не создает исключений.
    // . Нам все равно, произойдет ли сбой при отключении соединения, поскольку
    // в нашем случае это не критическая операция
    session->m_sock.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);

    // Remove session form the map of active sessions.
    // атем мы удаляем соответствующую запись
    std::unique_lock<std::mutex> lock(m_active_sessions_guard);
    auto it = m_active_sessions.find(session->m_id);
    if (it != m_active_sessions.end()) m_active_sessions.erase(it);
    lock.unlock();
    boost::system::error_code ec;
    if (session->m_ec && session->m_was_cancelled)
      ec = asio::error::operation_aborted;
    else
      ec = session->m_ec;
    // Call the callback provided by the user.
    // Кроме того, в качестве последнего шага вызывается предоставленный
    // пользователем обратный вызов. После возврата функции обратного вызова
    // жизненный цикл запроса завершается.
    session->m_callback(session->m_id, session->m_response, ec);
  };

 private:
  asio::io_service m_ios;
  std::map<int, std::shared_ptr<Session>> m_active_sessions;
  std::mutex m_active_sessions_guard;
  std::unique_ptr<boost::asio::io_service::work> m_work;
  std::unique_ptr<std::thread> m_thread;
};

// Завершение запроса – функция обратного вызова hendler().
// Для всех трех запросов, инициированных в функции main(),
// handler() указывается в качестве обратного вызова.
// Эта функция вызывается, когда запрос завершен,
// независимо от причины его завершения — будь то успешное завершение или
// ошибка. Также эта функция вызывается, когда запрос отменяется пользователем.
// Функция принимает три аргумента следующим образом:

// unsigned int request_id: содержит уникальный идентификатор запроса. Это тот
// же идентификатор, который был присвоен запросу при его инициализации.

// callvack function , print standard output stream if the request is completed
// successfully

/**
 * @brief
 *
 * @param request_id содержит уникальный идентификатор запроса. Это тот же
 * идентификатор, который был присвоен запросу при его инициализации.
 * @param response  Содержит данные ответа. Это значение считается
 * действительным только в том случае, если запрос успешно завершен и не отменен
 * @param ec Если во время жизненного цикла запроса возникает ошибка, этот
 * объект содержит информацию об ошибке. Если запрос был отменен, он содержит
 * значение asio::error::operation_aborted.
 */
void handler(unsigned int request_id, const std::string& response,
             const system::error_code& ec) {
  if (ec) {
    std::cout << "Request #" << request_id
              << " has completed. Response: " << response << std::endl;
  } else if (ec == asio::error::operation_aborted) {
    std::cout << "Request #" << request_id << " has been cancelled by the user."
              << std::endl;
  } else {
    std::cout << "Request #" << request_id
              << " failed! Error code = " << ec.value()
              << ". Error message = " << ec.message() << std::endl;
  }
  return;
}

// Запуск приложения – функция точки входа main()
// Функция main() вызывается в контексте потока пользовательского интерфейса.
// Эта функция имитирует поведение пользователя, который инициирует и отменяет
// запросы. Во-первых, он создает экземпляр класса AsyncTCPClient, а затем
// трижды вызывает его метод emulate Long Computation Op(), чтобы инициировать
// три асинхронных запроса, каждый раз указывая другой целевой сервер. Первый
// запрос (с присвоенным идентификатором 1) отменяется путем вызова метода
// cancel Request() через несколько секунд после инициализации запроса.

int main() {
  try {
    AsyncTCPClient client;
    // Here we emulate the user's behavior.
    // User initiates a request with id 1.
    client.emulateLongComputationOp(10, "127.0.0.1", 3333, handler, 1);
    // Then does nothing for 5 seconds.
    std::this_thread::sleep_for(std::chrono::seconds(5));
    // Then initiates another request with id 2.
    client.emulateLongComputationOp(11, "127.0.0.1", 3334, handler, 2);
    // Then decides to cancel the request with id 1.
    client.cancelRequest(1);
    // Does nothing for another 6 seconds.
    std::this_thread::sleep_for(std::chrono::seconds(6));
    // Initiates one more request assigning ID3 to it.
    client.emulateLongComputationOp(12, "127.0.0.1", 3335, handler, 3);
    // Does nothing for another 15 seconds.
    std::this_thread::sleep_for(std::chrono::seconds(15));
    // Decides to exit the application.
    client.close();
  } catch (system::system_error& e) {
    std::cout << "Error occured! Error code = " << e.code()
              << ". Message: " << e.what();
    return e.code().value();
  }
  return 0;
};