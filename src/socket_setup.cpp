#include "../include/socket_setup.hpp"

#include <bits/stdc++.h>
#include <arpa/inet.h>

// When program is ended with ctrl-c the socket gets closed and the loop ended.
void handle(int) {
    running = false;
}

sock_info setup(const int port) {
    // Create UDP socket
    const int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // setup handle function
    struct sigaction ctrl_c_handler;
    ctrl_c_handler.sa_handler = handle;
    sigemptyset(&ctrl_c_handler.sa_mask);
    sigaction(SIGINT, &ctrl_c_handler, NULL);

    // setup client connection
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr);
    return std::make_pair(sockfd, client_addr);
}
