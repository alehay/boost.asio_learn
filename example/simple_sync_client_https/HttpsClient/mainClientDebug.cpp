#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "./HttpsClient.hpp"

boost::json::object getJsonMesage() {
  boost::json::object loadToBucket;

  loadToBucket.emplace("name0 ", "value0");

  loadToBucket.emplace("name1", "value1");

  boost::json::array arr;
  arr.emplace_back("elem0");
  arr.emplace_back("elem1");
  arr.emplace_back("elem2");

  loadToBucket.emplace("arr", arr);

  return loadToBucket;
}

int main() {
  boost::asio::io_context ioContext;
  Config config;

  config.rootCACertificate = "./rootca.crt";
  config.serverPort = "127.0.0.1";
  config.serverPort = "65500";

  // auto message =  getUriToFile();
  auto message = getJsonMesage();

  // target http message
  std::string target = "/v1/unloading";

  // auto message =  getLoadToBucket();
  // std::string target = "/v1/load";

  // show json
  std::cout << "i send ->" << boost::json::serialize(message) << std::endl;

  // Create client to require data from ScreenRecorder API
  HttpsClient client(ioContext, config);

  bool isBad = true;

  ioContext.run();

  // send message in Post reauest
  std::string response =
      client.PostRequest(boost::json::serialize(message), target, isBad);

  // show response
  std::cout << " \n response ->" << response << std::endl;

  return 0;
}