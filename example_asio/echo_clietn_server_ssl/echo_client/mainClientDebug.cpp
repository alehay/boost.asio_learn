#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>


#include "./HttpsClient.hpp"



int main()
{


    std::cout << "start " << std::endl;
    boost::asio::io_context ioContext;
    Config config;
    config.rootCACertificate = "./cert/cert.pem";
    config.serverHost = "www.example.com";
    config.serverPort = "443";



    std::string target = "/";

    //auto message =  getLoadToBucket();
    //std::string target = "/v1/load";
  

    // Create client to require data from ScreenRecorder API
    HttpsClient client(ioContext, config);




    bool isBad = true; 


    ioContext.run();
    
//    std::string response = client.PostRequest(boost::json::serialize(message), target, isBad);
    std::string response = client.GetRequest();


    std::cout << " \n response ->" << response  << std::endl;
    
   
    



    return 0;
}