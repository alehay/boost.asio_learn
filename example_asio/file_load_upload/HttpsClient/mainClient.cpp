
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "./HttpsClient.hpp"


int main() {
  boost::asio::io_context ioContext {1};
  Config config;

  config.rootCACertificate = "./rootca.crt";
  config.serverHost = "127.0.0.1";
  config.serverPort = "65500";

  // auto message =  getUriToFile();

  // target http message
  std::string target = "/v1/unloading";

  // auto message =  getLoadToBucket();
  // std::string target = "/v1/load";

  // show json
  // std::cout << "i send ->" << boost::json::serialize(message) << std::endl;

  HttpsClient client(ioContext, config);

  bool isBad = true;

  ioContext.run();

  // send message in Post reauest
  std::string response =
      client.GetRequest("./load_file.txt", "./save_file.txt", isBad);

  return 0;
}