#include <bits/stdc++.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include "../include/socket_setup.hpp"

// When program is ended with ctrl-c the socket gets closed and the loop ended.
void handle(int) {
    running = false;
}

sock_info setup(const int port) {
    // Create UDP socket
#ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed with error: " << iResult << std::endl;
        exit(EXIT_FAILURE);
    }

    // Create UDP socket (Windows uses the SOCKET type)
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
#else
    const int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
#endif

#ifndef _WIN32
    // setup handle function
    struct sigaction ctrl_c_handler;
    ctrl_c_handler.sa_handler = handle;
    sigemptyset(&ctrl_c_handler.sa_mask);
    sigaction(SIGINT, &ctrl_c_handler, NULL);
#endif

    // setup client connection
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr);
    return std::make_pair(sockfd, client_addr);
}
