#include "HttpsClient.hpp"

HttpsClient::HttpsClient(boost::asio::io_context& context, Config& config)
    : context(context)
    , ssl_context(boost::asio::ssl::context::tlsv12_client)
    , resolver(context)
{
    std::cout << "start HttpsClient constructor" << std::endl;
    boost::system::error_code error;
    ssl_context.set_verify_mode(boost::asio::ssl::verify_peer, error);
    if (error.failed())
    {
        std::cout << "error set_verify_mode" << std::endl;
        exit(23);
    }


    ssl_context.load_verify_file(config.rootCACertificate, error);
    if (error.failed())
    {
        std::cout << "error load_verify_file root CA cert" << std::endl;
        exit(43);
    }

    results = resolver.resolve(config.serverHost, config.serverPort, error);
    if (error.failed())
    {
        std::cout << "error resolving host - check net or vpn" << std::endl;
        exit(53);
    }
}

std::string HttpsClient::PostRequest(const std::string& task, const std::string& target, bool& bad)
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
    if (error.failed())
    {
        bad = true;
        boost::beast::get_lowest_layer(stream).close();
        return error.message();
    }

    stream.handshake(boost::asio::ssl::stream_base::client, error);
    if (error.failed())
    {
        bad = true;
        boost::beast::get_lowest_layer(stream).close();
        return error.message();
    }

    boost::beast::http::request<boost::beast::http::string_body> req(boost::beast::http::verb::post, target, 11);
    req.set(boost::beast::http::field::host, "some_host");
    req.body() = task;
    req.content_length(req.body().size());
    req.keep_alive(false);
    req.prepare_payload();

    boost::beast::http::write(stream, req, error);
    if (error.failed())
    {
        bad = true;
        boost::beast::get_lowest_layer(stream).close();
        return error.message();
    }

    boost::beast::flat_buffer buffer;
    boost::beast::http::response<boost::beast::http::dynamic_body> res;
    boost::beast::http::read(stream, buffer, res, error);
    if (error.failed())
    {
        bad = true;
        boost::beast::get_lowest_layer(stream).close();
        return error.message();
    }

    boost::beast::get_lowest_layer(stream).close();

    if (res.result() != boost::beast::http::status::ok)
    {
        bad = true;
    }

    return boost::beast::buffers_to_string(res.body().data());
}