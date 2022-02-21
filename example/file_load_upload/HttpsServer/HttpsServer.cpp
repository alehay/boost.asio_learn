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

HttpsSession::HttpsSession(boost::asio::ip::tcp::socket&& socket, const std::string& filePath, 
                            boost::asio::ssl::context& context, std::weak_ptr<HttpsServer> host)
                                        : stream(std::move(socket), context)
                                        , filePath(filePath)
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

boost::beast::http::response<boost::beast::http::string_body> 
    HttpsServer::HandlePostFindWithMeta(boost::beast::http::request<boost::beast::http::string_body>&& req)
{
    std::string val = req.body();
    boost::system::error_code error;
    boost::json::value target = boost::json::parse(val, error);
    if(!error)
    {
        bool bad;
        auto result_find = ProcessRequest(target, bad);

        if(bad)
        {
            return Error(boost::beast::http::status::bad_request, "Error in JSON fields", req.version());
        }

        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::ok, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = boost::json::serialize(result_find);
        res.content_length(res.body().size());
        res.prepare_payload();

        return res;
    }
    return Error(boost::beast::http::status::bad_request, "Body is not json", req.version());
}

std::variant<boost::beast::http::response<boost::beast::http::file_body>,
                boost::beast::http::response<boost::beast::http::string_body>>
    HttpsServer::HandleGetLoad(boost::beast::http::request<boost::beast::http::string_body>&& req)
{
    std::string target = std::string(req.target().begin(), req.target().end());

    if(!std::filesystem::exists(target) || target.find(fileDir) == std::string::npos)
    {
        return Error(boost::beast::http::status::not_found, target, req.version());
    }

    boost::system::error_code ec;

    boost::beast::http::file_body::value_type body;
    body.open(target.c_str(), boost::beast::file_mode::scan, ec);
    if(ec.failed())
    {
        return Error(boost::beast::http::status::internal_server_error, "Can't open file: '" + target + "'", req.version());
    }

    auto size = body.size();

    boost::beast::http::response<boost::beast::http::file_body> res{
    std::piecewise_construct,
    std::make_tuple(std::move(body)),
    std::make_tuple(boost::beast::http::status::ok, req.version())};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type, GetContentType(target));
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

void HttpsSession::OnPerformingSsl(boost::system::error_code error)
{
    if(error)
    {
        spdlog::warn("Error handshake");
        return;
    }
    DoRead();
}

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

void HttpsSession::OnRead(boost::system::error_code error, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(error == boost::beast::http::error::end_of_stream)
    {
        return DoClose();
    }

    if(error)
    {
        spdlog::warn("Error on read");
    }

    HandleRequest(std::move(req), exec);
}

void HttpsSession::OnWrite(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        spdlog::warn(ec.message());
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
        spdlog::warn("Error on shutdown");
    }
}

HttpsServer::HttpsServer(const Config& config, const std::string& fileDir, const std::string InetIp, boost::asio::io_context& context)
    : ctx(boost::asio::ssl::context::tlsv12)
    , context(context)
    , acc(context)
    , fileDir(fileDir)
    , config(config)
{
    LoadServerCertificate();
    boost::asio::ip::tcp::endpoint end(boost::asio::ip::make_address(InetIp), port);
    boost::system::error_code error;

    acc.open(end.protocol(), error);
    if(error)
    {
        spdlog::critical("Error on open acceptor");
        exit(1);
    }

    acc.set_option(boost::asio::socket_base::reuse_address(true), error);
    if(error)
    {
        spdlog::critical("Error on set options in acceptor");
        exit(1);
    }

    acc.bind(end, error);
    if(error)
    {
        spdlog::critical("Error on bind acceptor's port");
        exit(1);
    }

    acc.listen(boost::asio::socket_base::max_listen_connections, error);
    if(error)
    {
        spdlog::critical("Error on listen");
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
        spdlog::critical("No " + config.currentServerCertificate);
        bad = true;
    }

    if(!std::filesystem::exists(config.currentServerKey))
    {
        spdlog::critical("No " + config.currentServerKey);
        bad = true;
    }

    if(!std::filesystem::exists(config.diffieHellman))
    {
        spdlog::critical("No " + config.diffieHellman);
        bad = true;
    }

    if(bad)
    {
        exit(1);
    }
    
    boost::system::error_code error;

    ctx.use_certificate_chain_file(config.currentServerCertificate, error);
    if(error.failed())
    {
        spdlog::critical("Certificate: " + error.message());
        exit(1);
    }

    ctx.use_private_key_file(config.currentServerKey, boost::asio::ssl::context::pem, error);
    if(error.failed())
    {
        spdlog::critical("Private key: " + error.message());
        exit(1);
    }

    ctx.use_tmp_dh_file(config.diffieHellman, error);
    if(error.failed())
    {
        spdlog::critical("Diffie-Hellman: " + error.message());
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
        spdlog::warn("Error on accept");
        std::this_thread::sleep_for(std::chrono::milliseconds(404));
    }
    else
    {
        std::make_shared<HttpsSession>(
        std::move(sock),
        fileDir,
        ctx,
        this->weak_from_this())->Run();
    }
    DoAccept();
}

boost::json::value HttpsServer::ProcessRequest(boost::json::value task, bool& bad)
{
    bad = false;

    auto task_obj = task.as_object();

    if (task_obj.empty()) 
    {
        spdlog::warn("Error, empty params");
        bad = true;
        return {"Empty json"};
    }

    if(task_obj.find("project_id") == task_obj.end())
    {
        spdlog::warn("Error, without project_id");
        bad = true;
        return {"Without project_id"};
    }

    if(task_obj.find("date_from") == task_obj.end())
    {
        spdlog::warn("Error, without date_from");
        bad = true;
        return {"Without date_from"};
    }

    if(task_obj.find("date_to") == task_obj.end())
    {
        spdlog::warn("Error, without date_to");
        bad = true;
        return {"Without date_to"};
    }

    RequestHandler requestHandler(config, fileDir);
    RequestHandler::User userData;

    // constructing a query by json fields in request
    for (auto&& val : task_obj) 
    {
        auto key = val.key();

        if (key == "date_from") 
        {
            if(!val.value().is_string())
            {
                spdlog::error("date_from - must be string");
                bad = true;
                return {"date_from - must be string"};
            }
            userData.date_from = val.value().as_string().c_str();
            continue;
        }

        if (key == "date_to") 
        {
            if(!val.value().is_string())
            {
                spdlog::error("date_to - must be string");
                bad = true;
                return {"date_to - must be string"};
            }
            userData.date_to = val.value().as_string().c_str();
            continue;
        }

        if (key == "theme") 
        {
            if(!val.value().is_string())
            {
                spdlog::error("theme - must be string");
                bad = true;
                return {"theme - must be string"};
            }
            userData.theme = val.value().as_string().c_str();
            continue;
        }

        if (key == "pin") 
        {
            if(!val.value().is_string())
            {
                spdlog::error("pin - must be string");
                bad = true;
                return {"pin - must be string"};
            }
            userData.pin = val.value().as_string().c_str();
            continue;
        }

        if (key == "project_id") 
        {
            if(!val.value().is_string())
            {
                spdlog::error("project_id - must be string");
                bad = true;
                return {"project_id - must be string"};
            }
            userData.project_id = val.value().as_string().c_str();
            continue;
        }

        if (key == "agent_id") 
        {
            if(!val.value().is_array())
            {
                spdlog::error("agent_id - must be array");
                bad = true;
                return {"agent_id - must be array"};
            }
            auto arr = val.value().as_array();
            for(auto&& agent_id : arr)
            {
                if(agent_id.is_string())
                {
                    userData.agent_id.emplace_back(agent_id.as_string().c_str());
                }
                else
                {
                    spdlog::error("agent_id in array - must be string");
                    bad = true;
                    return {"agent_id in array - must be string"};
                }
            }
            continue;
        }
    }

    auto res = requestHandler.SelectFileNames(userData);

    boost::json::array arr;
    for(auto&& val : res)
    {
        boost::json::object obj;
        if(!val.dateTimeUtc.empty())
        {
            obj["created_utc"] = val.dateTimeUtc;
        }
        if(!val.dateTimeGmt.empty())
        {
            obj["created_gmt"] = val.dateTimeGmt;
        }
        if(!val.theme.empty())
        {
            obj["theme"] = val.theme;
        }
        if(!val.pin.empty())
        {
            obj["pin"] = val.pin;
        }
        if(!val.project_id.empty())
        {
            obj["project_id"] = val.project_id;
        }
        if(!val.agent_id.empty())
        {
            obj["agent_id"] = val.agent_id;
        }
        if(!val.extention_number.empty())
        {
            obj["extension_number"]=val.extention_number;
        }
        if(val.display)
        {
            obj["display"] = val.display;
        }
        if(!val.fileName.empty())
        {
            obj["absolute_filename"] = val.fileName;
        }
        arr.emplace_back(obj);
    }

    return arr;
}

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
    if(req.method() == boost::beast::http::verb::post)
    {
        send(std::move(host.lock()->HandlePostFindWithMeta(std::move(req))));
        
        return;
    }

    spdlog::warn("Unknown HTTP-method");
    send(std::move(Error(boost::beast::http::status::bad_request,"Unknown HTTP-method", req.version())));
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