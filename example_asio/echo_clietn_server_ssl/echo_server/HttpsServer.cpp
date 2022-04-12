#include "HttpsServer.hpp"

std::string HttpsSession::GetContentType(const std::string& target)
{
    if(target.find(".mp4") != std::string::npos){
        return "video/mp4";
    }
    if(target.find(".mp3") != std::string::npos){
        return "audio/mpeg";
    }
    return "application/text";
}

HttpsSession::HttpsSession(boost::asio::ip::tcp::socket&& socket, const std::string& filePath, 
                            boost::asio::ssl::context& context )
                            //std::weak_ptr<HttpsServer> host)
                                        : stream(std::move(socket), context)
                                        , filePath(filePath)
                                        , exec(*this)
                                        // , host(host)
{

}

void HttpsSession::Run()
{
    #ifdef DEBUG_PRINT//  
         std::cout << "20 HttpsSession RUN !" << std::endl;  
    #endif
    boost::asio::dispatch(
            stream.get_executor(),
            boost::beast::bind_front_handler(
                &HttpsSession::OnRun,
                this->shared_from_this()));
    #ifdef DEBUG_PRINT//  
         std::cout << "20 HttpsSession END !" << std::endl;  
    #endif
}

void HttpsSession::OnRun()
{
    #ifdef DEBUG_PRINT//  
         std::cout << "30 Https Session ON RUN !" << std::endl;  
    #endif
    // 
    boost::beast::get_lowest_layer(stream).expires_after(
            std::chrono::seconds(30));
    stream.async_handshake(
        boost::asio::ssl::stream_base::server,
        boost::beast::bind_front_handler(
            &HttpsSession::OnPerformingSsl,
       
            this->shared_from_this()));
    #ifdef DEBUG_PRINT//
         std::cout << "30 Https Session ON RUN END !" << std::endl;  
    #endif
}

void HttpsSession::OnPerformingSsl(boost::system::error_code error)
{
    #ifdef DEBUG_PRINT//  
         std::cout << " 50  HttpsSession::OnPerformingSsl" << std::endl;  
    #endif
    if(error)
    {
        // spdlog::warn("Error handshake");
        std::cout << "ERROR HANDSHAKE " << std::endl;
    } else {
        std::cout << "Handshake OK !!!" << std::endl;
    }
    DoRead();
    #ifdef DEBUG_PRINT//   
         std::cout << " 50 HttpsSession::OnPerformingSsl END" << std::endl;  
    #endif
}

void HttpsSession::DoRead()
{
    #ifdef DEBUG_PRINT//   
         std::cout << "60 HttpsSession::DoRead" << std::endl;  
    #endif
    req = {};
    boost::beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Read a request
    boost::beast::http::async_read(stream, buff, req,
        boost::beast::bind_front_handler(
            &HttpsSession::OnRead,
            this->shared_from_this()));
    #ifdef DEBUG_PRINT//   
         std::cout << "60 HttpsSession::DoRead END" << std::endl;  
    #endif
}

void HttpsSession::OnRead(boost::system::error_code error, std::size_t bytes_transferred)
{
    #ifdef DEBUG_PRINT//   
         std::cout << "70 HttpsSession:: On Read" << std::endl;  
    #endif
    boost::ignore_unused(bytes_transferred);
    if(error == boost::beast::http::error::end_of_stream)
    {
        return DoClose();
    }
    if(error)
    {
        //spdlog::warn("Error on read");
    }
    #ifdef DEBUG_PRINT//   
         std::cout << "HandleRequest Calles run Executable" << std::endl;  
    #endif
    HandleRequest(this->filePath, std::move(req), exec);
    #ifdef DEBUG_PRINT//   
         std::cout << " 70 HttpsSession:: On Read END" << std::endl;  
    #endif
}

void HttpsSession::OnWrite(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        // spdlog::warn(ec.message());
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
        // spdlog::warn("Error on shutdown");
    }
}

HttpsServer::HttpsServer(const Config& config_, 
                         const std::string InetIp, 
                         boost::asio::io_context& context)
    : ctx(boost::asio::ssl::context::tlsv12)
    , context(context)
    , acc(context)
    , fileDir(fileDir)
    , config(config_)
{
    std::cout << "HTTP concstructor " << std::endl;
    port = std::stoul(config.port);
    LoadServerCertificate();
    boost::asio::ip::tcp::endpoint end(boost::asio::ip::make_address(InetIp), port);
    boost::system::error_code error;
    std::cout << "load parameters " << std::endl;

    acc.open(end.protocol(), error);
    if(error)
    {
       // spdlog::critical("Error on open acceptor");
        return;
    }
    acc.set_option(boost::asio::socket_base::reuse_address(true), error);
    if(error)
    {
        // spdlog::critical("Error on set options in acceptor");
        return;
    }
    acc.bind(end, error);
    if(error)
    {
        // spdlog::critical("Error on bind acceptor's port");
        return;
    }
    acc.listen(boost::asio::socket_base::max_listen_connections, error);
    if(error)
    {
        // spdlog::critical("Error on listen");
        return;
    }
    std::cout << "HTTP constructor END" << std::endl;
}

HttpsServer::~HttpsServer()
{
    acc.close();
}

void HttpsServer::Run()
{
    DoAccept();
}

void HttpsServer::LoadServerCertificate()
{
    std::cout << "start Load Sert" << std::endl;
    ctx.set_options(
			boost::asio::ssl::context::default_workarounds
			| boost::asio::ssl::context::no_sslv2
			| boost::asio::ssl::context::single_dh_use);
		// ctx.set_password_callback(std::bind(&HttpsServer::GetPassword, this));
		ctx.use_certificate_chain_file("./server01.crt");
		ctx.use_private_key_file("./server01.key", boost::asio::ssl::context::pem);
		ctx.use_tmp_dh_file("./dh2048.pem");
    std::cout << "end load sert" << std::endl;
}

std::string HttpsServer::GetPassword()const
{
    return "123456";
}

void HttpsServer::DoAccept()
{
    #ifdef DEBUG_PRINT
        std::cout << "Https Do Accent " << std::endl;
    #endif

    acc.async_accept(
        boost::asio::make_strand(context),
        boost::beast::bind_front_handler(
            &HttpsServer::OnAccept,
            this->shared_from_this()));

    #ifdef DEBUG_PRINT
        std::cout << "Https Do Accent END" << std::endl;
    #endif

}

void HttpsServer::OnAccept(boost::beast::error_code error, boost::asio::ip::tcp::socket sock)
{
    #ifdef DEBUG_PRINT   
        std::cout << "10 On Accep START !" << std::endl;  
        std::cout << "Connection start! " << std::endl;
    #endif
    if(error)
    {
        //spdlog::warn("Error on accept");
        std::cout << "error On Accept" << std::endl; 
        std::this_thread::sleep_for(std::chrono::milliseconds(404));
    }
    else
    {
        std::make_shared<HttpsSession>(
        std::move(sock),
        fileDir,
        ctx) -> Run() ;
         //this->weak_from_this())->Run() ;
    }
 
    DoAccept();
    #ifdef DEBUG_PRINT//   
         std::cout << "10 On Accep EDN !" << std::endl;  
    #endif
}

boost::json::value HttpsSession::ProcessRequest(boost::json::value task)
{
    auto task_obj = task.as_object();
   
   std::cout << "!!HttpsSession::ProcessRequest" << std::endl;
   std::cout << task_obj << std::endl;
   /*
    if (task_obj.empty()) {
        spdlog::warn("Error, empty params");
        return {"Empty json"};
    }
    if(task_obj.find("login") == task_obj.end())
    {
        spdlog::warn("Error, without login");
        return {"Without login"};
    }
    if(task_obj.find("date_from") == task_obj.end())
    {
        spdlog::warn("Error, without date_from");
        return {"Without date_from"};
    }
    if(task_obj.find("date_to") == task_obj.end())
    {
        spdlog::warn("Error, without date_to");
        return {"Without date_to"};
    }
    */
   // RequestHandler requestHandler(config, fileDir);
   // RequestHandler::User userData;

    // constructing a query by json fields in request
    for (auto&& val : task_obj) {
        auto key = val.key();


        /*
        if (key == "login") {
            if(!val.value().is_string()){
                spdlog::error("login - must be string");
                return {"login - must be string"};
            }
            std::string login = val.value().as_string().c_str();

            auto result = std::find(privilegedUser.begin(), privilegedUser.end(), login);
            if (result != privilegedUser.end()) {
                userData.login = login;
            } else {
                spdlog::error("Person without access: " + login);
                return {"Person without access"};
            }
            continue;
        }
        */

        /*
        if (key == "date_from") {
            if(!val.value().is_string()){
                spdlog::error("date_from - must be string");
                return {"date_from - must be string"};
            }
            userData.date_from = val.value().as_string().c_str();
            continue;
        }

        if (key == "date_to") {
            if(!val.value().is_string()){
                spdlog::error("date_to - must be string");
                return {"date_to - must be string"};
            }
            userData.date_to = val.value().as_string().c_str();
            continue;
        }

        if (key == "theme") {
            if(!val.value().is_string()){
                spdlog::error("theme - must be string");
                return {"theme - must be string"};
            }
            userData.theme = val.value().as_string().c_str();
            continue;
        }

        if (key == "pin") {
            if(!val.value().is_string()){
                spdlog::error("pin - must be string");
                return {"pin - must be string"};
            }
            userData.pin = val.value().as_string().c_str();
            continue;
        }

        if (key == "project_id") {
            if(!val.value().is_string()){
                spdlog::error("project_id - must be string");
                return {"project_id - must be string"};
            }
            userData.project_id = val.value().as_string().c_str();
            continue;
        }

        if (key == "agent_id") {
            if(!val.value().is_string()){
                spdlog::error("agent_id - must be string");
                return {"agent_id - must be string"};
            }
            userData.agent_id = val.value().as_string().c_str();
            continue;
        }
        */
    }
   // std::string res = requestHandler.SelectFileNames(userData);
   
   std::string res = " pong ";
    boost::json::stream_parser p;
    boost::json::error_code error;
    p.write(res, error);
    if(!error){
        p.finish(error);
        if(!error)
        {
            boost::json::value val;
            try
            {
                val = p.release();
            }
            catch(...)
            {
                return {};
            }
            return val;
        }
        return {};
    }
    return {};
}