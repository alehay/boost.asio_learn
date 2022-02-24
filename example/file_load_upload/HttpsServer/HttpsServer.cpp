#include "HttpsServer.hpp"

std::string HttpsServer::GetContentType(const std::string& target)
{
    if(target.find(".mp4") != std::string::npos){
        return "video/mp4";
    }

    if(target.find(".mp3") != std::string::npos){
        return "audio/mpeg";
    }

    return "application/text";
}

HttpsSession::HttpsSession(
                            boost::asio::ip::tcp::socket&& socket, 
                            boost::asio::ssl::context& context, 
                            std::weak_ptr<HttpsServer> host)
                                        : stream(std::move(socket), context)
                                        , exec(*this)
                                        , host(host)
{

}

boost::beast::http::response<boost::beast::http::string_body> 
        HttpsServer::Error(boost::beast::http::status status, const std::string& what,unsigned version)
{
    boost::beast::http::response<boost::beast::http::string_body> res{status, version};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(false);
    res.body() = "An error occurred: '" + what + "'";
    res.prepare_payload();

    return res;
}



std::variant<boost::beast::http::response<boost::beast::http::file_body>,
                boost::beast::http::response<boost::beast::http::string_body>>
    HttpsServer::HandleGetLoad(boost::beast::http::request<boost::beast::http::string_body>&& req)
{
    std::cout << "hendl get has ben started " << std::endl;
    std::string target = std::string(req.target().begin(), req.target().end());
    std::cout << "target is " << target << std::endl;

    std::string loadTarget {"/v1/download"};

    std::string fileName = target.substr(loadTarget.size());

    std::cout << "file name is " << fileName << std::endl;

    if(!std::filesystem::exists(fileName) )
    {
        std::cout << "file " << target << "error !" << std::endl;
        return Error(boost::beast::http::status::not_found, target, req.version());
    }

    boost::system::error_code ec;

    boost::beast::http::file_body::value_type body;
    body.open(fileName.c_str(), boost::beast::file_mode::scan, ec);
    if(ec.failed())
    {
        return Error(boost::beast::http::status::internal_server_error, "Can't open file: '" + fileName + "'", req.version());
    }

    auto size = body.size();
    std::cout << "body.size() = " << size << std::endl; 

    boost::beast::http::response<boost::beast::http::file_body> res{
    std::piecewise_construct,
    std::make_tuple(std::move(body)),
    std::make_tuple(boost::beast::http::status::ok, req.version())};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, GetContentType(fileName)); // ? 
    res.content_length(size);
    res.keep_alive(req.keep_alive());

    return res;
}



void HttpsSession::Run()
{
    boost::asio::dispatch(
            stream.get_executor(),
            boost::beast::bind_front_handler(
                &HttpsSession::OnRun,
                this->shared_from_this()));
}
// run call -> on run 


void HttpsSession::OnRun()
{
    boost::beast::get_lowest_layer(stream).expires_after(
            std::chrono::seconds(30));

    stream.async_handshake(
        boost::asio::ssl::stream_base::server,
        boost::beast::bind_front_handler(
            &HttpsSession::OnPerformingSsl,
            this->shared_from_this()));
}
// OnRun -> OnPerformingSsl


void HttpsSession::OnPerformingSsl(boost::system::error_code error)
{
    if(error)
    {
        std::cout << "Error handshake" << std::endl;
        std::cout << error.what() << std::endl;
        std::cout << error.message() << std::endl;
        std::cout << error.category().name() << std::endl;
        return;
    }
    DoRead();
}
// OnPerformingSsl -> DoRead

void HttpsSession::DoRead()
{
    req = {};
    boost::beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Read a request
    boost::beast::http::async_read(stream, buff, req,
        boost::beast::bind_front_handler(
            &HttpsSession::OnRead,
            this->shared_from_this()));
}
// DoRead -> OnRead

void HttpsSession::OnRead(boost::system::error_code error, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(error == boost::beast::http::error::end_of_stream)
    {
        return DoClose();
    }

    if(error)
    {
        std::cout << "Error on read" << std::endl;
    }

    HandleRequest(std::move(req), exec);
}
// OnRead -> HandleRequest

void HttpsSession::HandleRequest(boost::beast::http::request<boost::beast::http::string_body>&& req, Executable& send)
{
    if(req.method() == boost::beast::http::verb::get)
    {
        auto res = host.lock()->HandleGetLoad(std::move(req));

        if(res.index() == 0)
        {
            send(std::move(std::get<boost::beast::http::response<boost::beast::http::file_body>>(std::move(res))));
        }
        else
        {
            send(std::move(std::get<boost::beast::http::response<boost::beast::http::string_body>>(std::move(res))));
        }

        return;
    }

    std::cout << "Unknown HTTP-method" << std::endl;
    send(std::move(Error(boost::beast::http::status::bad_request,"Unknown HTTP-method", req.version())));
}


void HttpsSession::OnWrite(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        std::cout << ec.message()  << std::endl;
    }

    if(close)
    {
        return DoClose();
    }

    mg = nullptr;

    DoRead();
}

HttpsSession::Executable::Executable(HttpsSession& rf)
    : self(rf)
{

}

void HttpsSession::DoClose()
{
    boost::beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    stream.async_shutdown(
        boost::beast::bind_front_handler(
            &HttpsSession::OnShutdown,
            this->shared_from_this()));
}

void HttpsSession::OnShutdown(boost::beast::error_code error)
{
    if(error)
    {
        std::cout << "Error on shutdown" << std::endl;
    }
}

// ###############################  SERVER



HttpsServer::HttpsServer(const ConfigServer& conf,  const std::string InetIp, boost::asio::io_context& context)
    : ctx(boost::asio::ssl::context::tlsv12)
    , context(context)
    , acc(context)
    , config(conf)
{
    LoadServerCertificate();
    boost::asio::ip::tcp::endpoint end(boost::asio::ip::make_address(InetIp), std::stoul(config.serverPort));
    boost::system::error_code error;

    acc.open(end.protocol(), error);
    if(error)
    {
        std::cout << "Error on open acceptor" << std::endl;
        exit(1);
    }

    acc.set_option(boost::asio::socket_base::reuse_address(true), error);
    if(error)
    {
        std::cout << "Error on set options in acceptor" <<  std::endl;
        exit(1);
    }

    acc.bind(end, error);
    if(error)
    {
        std::cout << "Error on bind acceptor's port" << std::endl;
        exit(1);
    }

    acc.listen(boost::asio::socket_base::max_listen_connections, error);
    if(error)
    {
        std::cout << "Error on listen" << std::endl;
        exit(1);
    }
}

HttpsServer::~HttpsServer()
{
    acc.close();
}

void HttpsServer::Run()
{
    DoAccept();
}

// https://www.boost.org/doc/libs/1_73_0/doc/html/boost_asio/reference/ssl__context.html

void HttpsServer::LoadServerCertificate()
{
    ctx.set_options(
			boost::asio::ssl::context::default_workarounds
			| boost::asio::ssl::context::no_sslv2
			| boost::asio::ssl::context::single_dh_use);
		ctx.set_password_callback(std::bind(&HttpsServer::GetPassword, this));
    bool bad = false;

    if(!std::filesystem::exists(config.currentServerCertificate))
    {
        std::cout << "No " <<  config.currentServerCertificate << std::endl;
        bad = true;
    }

    if(!std::filesystem::exists(config.currentServerKey))
    {
        std::cout << "No " + config.currentServerKey << std::endl;
        bad = true;
    }

    if(!std::filesystem::exists(config.diffieHellman))
    {
        std::cout << "No " + config.diffieHellman << std::endl;
        bad = true;
    }

    if(bad)
    {
        exit(1);
    }
    
    boost::system::error_code error;

    // а если здесь не цепочка ??? 
    
    ctx.use_certificate_chain_file(config.currentServerCertificate, error);
    if(error.failed())
    {
        std::cout << "Certificate: " + error.message() << std::endl;
        exit(1);
    }

    ctx.use_private_key_file(config.currentServerKey, boost::asio::ssl::context::pem, error);
    if(error.failed())
    {
        std::cout << "Private key: " + error.message() << std::endl;
        exit(1);
    }

    ctx.use_tmp_dh_file(config.diffieHellman, error);
    if(error.failed())
    {
        std::cout << "Diffie-Hellman: " + error.message() << std::endl;
        exit(1);
    }
}

std::string HttpsServer::GetPassword()const
{
    return "123456";
}

void HttpsServer::DoAccept()
{
    acc.async_accept(
        boost::asio::make_strand(context),
        boost::beast::bind_front_handler(
            &HttpsServer::OnAccept,
            this->shared_from_this()));
}

void HttpsServer::OnAccept(boost::beast::error_code error, boost::asio::ip::tcp::socket sock)
{
    if(error)
    {
        std::cout << "Error on accept" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(404));
    }
    else
    {
        std::make_shared<HttpsSession>(
        std::move(sock),
        ctx,
        this->weak_from_this())->Run();
    }
    DoAccept();
}





boost::beast::http::response<boost::beast::http::string_body> HttpsSession::Error(boost::beast::http::status status, const std::string& what, unsigned version)
{
    boost::beast::http::response<boost::beast::http::string_body> res{status, version};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(false);
    res.body() = "An error occurred: '" + what + "'";
    res.prepare_payload();
    return res;
}