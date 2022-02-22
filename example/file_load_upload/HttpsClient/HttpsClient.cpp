#include "HttpsClient.hpp"

// https://www.boost.org/doc/libs/1_73_0/doc/html/boost_asio/reference/ssl__context.html

HttpsClient::HttpsClient(boost::asio::io_context& context, Config conf)
    : context(context)
    , ssl_context(boost::asio::ssl::context::tlsv12_client)
    , resolver(context)
{
    boost::system::error_code error;
   
   
    ssl_context.set_verify_mode(boost::asio::ssl::verify_peer, error);
   
   
    if(error.failed())
    {
        std::cout << "verify mode error"  << std::endl;
        exit(23);
    }

    if(!std::filesystem::exists(conf.rootCACertificate))
    {
        std::cout << "No filepath" <<  std::endl;
        exit(33);
    }


    // Здесь не каталог !!!! а файл !!!
    ssl_context.add_verify_path(conf.rootCACertificate, error);

    if(error.failed())
    {
        std::cout << "No verify path" << std::endl;
        exit(43);
    }
    
    results = resolver.resolve(conf.serverHost, conf.serverPort, error);
    if(error.failed())
    {
        std::cout << "Error resolving dns name" << std::endl;
         std::cout << error.what() << std::endl;
        std::cout << error.message() << std::endl;
        std::cout << error.category().name()  << std::endl;
        exit(53);
    }
}

/*
std::string HttpsClient::PostRequest(const std::string& task, bool& bad)
{   

    bad = false;
    boost::system::error_code error;
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream(context, ssl_context);

    stream.set_verify_callback([](bool preverified,
        boost::asio::ssl::verify_context& ctx)
        {
            return preverified;
        });

    boost::beast::get_lowest_layer(stream).connect(results, error);
    if(error.failed())
    {
        bad = true;
        std::cout << "Error connnecting" << std::endl;
        boost::beast::get_lowest_layer(stream).close();
        return error.message();
    }
    stream.handshake(boost::asio::ssl::stream_base::client, error);
    if(error.failed())
    {
        bad = true;
        std::cout << "Error handshaking" << std::endl;
        boost::beast::get_lowest_layer(stream).close();
        return error.message();
    }

    // specify target for massive unloading /v1/unloading/
    boost::beast::http::request<boost::beast::http::string_body> req(boost::beast::http::verb::post, "/v1/unloading", 11);
    req.set(boost::beast::http::field::host, "some_host");
    req.body() = task;
    req.content_length(req.body().size());
    req.keep_alive(false);
    req.prepare_payload();

    boost::beast::http::write(stream, req, error);
    if(error.failed())
    {
        bad = true;
        std::cout << "Error writing data to the socket" << std::endl;
        boost::beast::get_lowest_layer(stream).close();
        return error.message();
    }

    boost::beast::flat_buffer buffer;
    boost::beast::http::response<boost::beast::http::dynamic_body> res;
    boost::beast::http::read(stream, buffer, res, error);
    if(error.failed())
    {
        bad = true;
        std::cout << "Error reading from socket" << std::endl;
        boost::beast::get_lowest_layer(stream).close();
        return error.message();
    }

    boost::beast::get_lowest_layer(stream).close();

    if(res.result() != boost::beast::http::status::ok)
    {
        std::cout << "Beast response is not ok" << std::endl;
        bad = true;
    }

    return boost::beast::buffers_to_string(res.body().data());
}
*/

std::string HttpsClient::GetRequest(const std::string& filePath, const std::string& saveFilePath, bool& bad)
{
    bad = false;
    boost::system::error_code error;

    boost::beast::ssl_stream<boost::beast::tcp_stream> stream(context, ssl_context);

    stream.set_verify_callback([](bool preverified,
        boost::asio::ssl::verify_context& ctx)
        {
            return preverified;
        });

    boost::beast::get_lowest_layer(stream).connect(results, error);
    if(error.failed())
    {
        bad = true;
        std::cout << "Error connnecting" << std::endl;
        boost::beast::get_lowest_layer(stream).close();
        return error.message();
    }

    stream.handshake(boost::asio::ssl::stream_base::client, error);
    
    if(error.failed())
    {

        bad = true;
        std::cout <<  "Error handshaking" << std::endl;
        boost::beast::get_lowest_layer(stream).close();
        
        std::cout << error.what() << std::endl;
        std::cout << error.message() << std::endl;
        std::cout << error.category().name() << std::endl;


        return error.message();
    }

    // requests a file by path on the server
    boost::beast::http::request<boost::beast::http::empty_body> req(boost::beast::http::verb::get, "/v1/download" + filePath, 11);
   
    req.set(boost::beast::http::field::host, "some_host");
    req.keep_alive(false);
    req.prepare_payload();

    boost::beast::http::write(stream, req, error);
    if(error.failed())
    {
        bad = true;
        std::cout << "Error writing data to the socket" << std::endl;
        boost::beast::get_lowest_layer(stream).close();
        return error.message();
    }

    boost::beast::flat_buffer buffer;
    boost::beast::http::response<boost::beast::http::file_body> res;
    
    // open file to write data
    res.body().open(saveFilePath.c_str(), boost::beast::file_mode::write_new, error);
    
    if(error.failed())
    {
        bad = true;
        return error.message();
    }

    boost::beast::http::read(stream, buffer, res, error);
    if(error.failed())
    {
        bad = true;
        std::cout << "Error reading from socket" << std::endl;
        boost::beast::get_lowest_layer(stream).close();
        res.body().close();

        // femove file if error 
        std::filesystem::remove(saveFilePath);
        return error.message();
    }

    boost::beast::get_lowest_layer(stream).close();

    if(res.result() != boost::beast::http::status::ok)
    {
        std::cout << "api-screen-recorder-server response is not ok" << std::endl ;
        bad = true;
        res.body().close();

        //remove file if error 
        std::filesystem::remove(saveFilePath);
        return "api-screen-recorder-server response is not ok";
    }

    res.body().close();
    return std::string();
}