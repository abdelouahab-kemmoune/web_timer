#include <iostream>
#include <ctime>
#include <chrono>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <vector>
#include <cerrno>
#include <cstring>

int main() {

    std::cout << "Configuring local address...\n";
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *bind_address;
    getaddrinfo(0, "8080", &hints, &bind_address);

    std::cout << "Creating socket...\n";
    int socket_listen;
    socket_listen = socket(bind_address->ai_family,
    bind_address->ai_socktype, bind_address->ai_protocol);

    if (socket_listen < 0) {
        std::cerr << "socket() failed. Error: " << std::strerror(errno) 
              << " (" << errno << ")" << "\n";
    return 1;
    }

    std::cout << "Binding socket to local address...\n";
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        std::cerr << "bind() failed. \n" << std::strerror(errno)
            << " (" << errno << ")" << "\n";
        return 1;
    }
    freeaddrinfo(bind_address);

    std::cout << "Listening...\n";
    if (listen(socket_listen, 10) < 0) {
        std::cerr << "listen() failed. \n" << std::strerror(errno)
            << " (" << errno << ")" << "\n";
        return 1;
    }

    std::cout << "Waiting for connection...\n";
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    int socket_client = accept(socket_listen,
                        (struct sockaddr*) &client_address, &client_len);
    if (socket_client < 0) {
        std::cerr << "accept() failed. \n" << std::strerror(errno)
            << " (" << errno << ")" << "\n";
    return 1;
    }

    std::cout << "Client is connected...\n";
    char address_buffer[100];
    getnameinfo((struct sockaddr*)&client_address,
                client_len, address_buffer, sizeof(address_buffer), 0, 0,
                NI_NUMERICHOST);
    std::cout << address_buffer << "\n";

    std::cout << "Reading request...\n";
    char request [1024];
    int bytes_received = recv(socket_client, request, 1024, 0);
    std::cout << "Received " << bytes_received << "bytes.\n";

    std::cout << "Sending response...\n";
    const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Connection: close\r\n"
            "Content-Type: text/plain\r\n\r\n"
            "Local time is: ";
    int bytes_sent = send(socket_client, response, strlen(response), 0);
    std::cout << "Sent " << bytes_sent << "of " << std::strlen(response) << " bytes.\n";

    time_t timer;
    std::time(&timer);

    char *time_msg = ctime(&timer);
    bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
    std::cout << "Sent " << bytes_sent << " of " << std::strlen(time_msg) << " bytes.\n";

    std::cout << "Closing connection...\n";
    close(socket_client);

    std::cout << "Finished.\n";
    return 0;
}