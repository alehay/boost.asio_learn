#include <boost/asio.hpp>
#include <iostream>
using namespace boost;


// определяем структуру данных, содержащую указатель 
// на объект сокета, буфер, содержащий данные, 
// подлежащие записи, и переменную счетчика,
// содержащую количество уже записанных байтов:

// Сохраняет объекты, которые нам нужны 
// при обратном вызове, чтобы определить, 
// все ли данные были записаны в сокет, 
// и при необходимости инициировать следующую 
// операцию асинхронной записи.

struct Session {
    std::shared_ptr<asio::ip::tcp::socket> sock;
    std::string buf;
    std::size_t total_bytes_written;
};

/* вариант с использованием функции asio::async_write() */
struct Session_1 {
    std::shared_ptr<asio::ip::tcp::socket> sock;
    std::string buf;
};



// определяем функцию обратного вызова, 
// которая будет вызвана после 
// завершения асинхронной операции

// Функция, используемая в качестве обратного 
// вызова для операции асинхронной записи. 
// Проверяет, все ли данные из буфера были записаны в сокет, 
// и при необходимости инициирует новую операцию асинхронной записи.

void callback(const boost::system::error_code& ec,
    // При вызове функция обратного вызова
    
    std::size_t bytes_transferred,
    std::shared_ptr<Session> s) {

    // первым делом проверяем ошибку 
    if (ec) {
        std::cout << "Error occured! Error code = "
        << ec.value()
        << ". Message: " << ec.message();
    return;
    }
    // счетчик общего количества записанных байтов увеличивается на количество байтов, 
    // записанных в результате операции.
    s->total_bytes_written += bytes_transferred;
    
    // Затем мы проверяем, равно ли общее количество байтов, 
    // записанных в сокет, размеру буфера.
    if (s->total_bytes_written == s->buf.length()) {
        // если равны, занчи уже все записано 
        return;
    }

    //async_write_some() не блокирует поток выполнения. 
    // Он инициирует операцию записи и return .
    s->sock->async_write_some(
        asio::buffer(
            // начало буфера сдвигается на количество записанных байтов. 
            s->buf.c_str() + s->total_bytes_written,
            // а размер буфера уменьшается на тоже значение 
            s->buf.length() - s->total_bytes_written
        ),
        //  вызываем туже фнукицю ! с темже объктом сессии
        std::bind(callback, 
            std::placeholders::_1, 
            std::placeholders::_2, 
            s)
        );

    // получается цикл асинхронной записи , который повторяется пока 
    // все данные не будут записаны, или не произойдет ошибка 

    // кога фукнция возвращается без иницци 
}


/* вариант с использованием функции asio::async_write() */

void callback_1(const boost::system::error_code& ec,
        std::size_t bytes_transferred,
        std::shared_ptr<Session> s) {
    
    if (ec) {
        std::cout << "Error occured! Error code = "
        << ec.value()
        << ". Message: " << ec.message();
        return;
    }
// Here we know that all the data has
// been written to the socket.
}



void writeToSocket(std::shared_ptr<asio::ip::tcp::socket> sock) {
    std::shared_ptr<Session> s(new Session);
    
    s->buf = std::string("Hello");
    s->total_bytes_written = 0;
    s->sock = sock;


    // Step 5. Initiating asynchronous write operation.
    s->sock->async_write_some(
    // Первый аргумент - это буфер, содержащий данные для записи в сокет.
    // Поскольку операция является асинхронной, 
    // этот буфер может быть доступен с помощью Boost.Asio 
    // в любой момент между началом операции и вызовом обратного вызова.
    asio::buffer(s->buf),

    //Второй аргумент - это обратный вызов, 
    // который должен быть вызван после завершения асинхронной операции.
    std::bind(callback,
        std::placeholders::_1,
        std::placeholders::_2,
        s )

    );
}


int main() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 8085;
    
    try {
        asio::ip::tcp::endpoint
        ep(asio::ip::address::from_string(raw_ip_address),port_num);
        asio::io_service ios;
        // Step 3. Allocating, opening and connecting a socket.
        std::shared_ptr<asio::ip::tcp::socket> sock(
        new asio::ip::tcp::socket(ios, ep.protocol()));
        sock->connect(ep);
        writeToSocket(sock);
        // Step 6.
        ios.run();
    }
    catch (system::system_error &e) {
        std::cout << "Error occured! Error code = " << e.code()
        << ". Message: " << e.what();
        return e.code().value();
    }
    return 0;
}