// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>

#include <string>

#include <arpa/inet.h>
#define PORT 8081

std::string getIpAdress (int soc) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(soc, (struct sockaddr *)&addr, &addr_size);
    if (res < 0)
    {
        
    }
    return inet_ntoa(addr.sin_addr);
}


int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    const char *hello = "Hello from server";
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
       
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
       
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    valread = read( new_socket , buffer, 1024);
    printf("%s\n",buffer );
    send(new_socket , hello , strlen(hello) , 0 );
    printf("Server sent message \n");

   
    std::cout << "IP ADRESS IS - " <<  getIpAdress(new_socket) << std::endl;


    return 0;
}

/*
len = sizeof addr;
getpeername(p3_socket, (struct sockaddr*) &addr, &len);
struct sockaddr_in *s = (struct sockaddr_in *) &addr;
inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);

if (strcmp(ipstr, "127.0.0.1") == 0) {
    len = sizeof(addr);
    getsockname(p1_socket, (struct sockaddr *)addr, &len);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
}
*/

/*
int sockfd;

void main(void) {
    //[...]
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_size = sizeof(clientaddr);
    int newfd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientaddr_size);
    //fork() and other code
    foo(newfd);
    //[...]
}
void foo(int newfd) {
    //[...]
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(newfd, (struct sockaddr *)&addr, &addr_size);
    char *clientip = new char[20];
    strcpy(clientip, inet_ntoa(addr.sin_addr));
    //[...]
} */