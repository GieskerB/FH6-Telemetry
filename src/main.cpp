#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <iostream>

#include "../include/fh6_data.hpp"
#include "../include/socket_setup.hpp"

// Running variable to stop loop when program ends.
volatile bool running = true;

// Small wrapper around recvfrom
ssize_t receive_message(int sockfd, void* message, const struct sockaddr* client_addr) {
    static socklen_t len = sizeof(struct sockaddr);
    return recvfrom(sockfd, message, TELEMETRY_SIZE, MSG_WAITALL, (struct sockaddr *)&client_addr, &len);
}

// Continuously receive data via UDP.
void receive_loop(int sockfd, const struct sockaddr* client_addr) {
    for (int i = 0; i < 1000; ++i) {
        struct fh6_data dummy_data;

        // Call wrapper, exit if data could not be received.
        if (receive_message(sockfd, ((void*) &dummy_data), (const struct sockaddr*)&client_addr) < 0) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }

        printf("%d\n", dummy_data.IsRaceOn);
    }
    close(sockfd);
}

int main(int argc, const char* argv[]) {

    if(argc != 2) {
        perror("UPD test server requires one argument:\n\tmessage port\n");
        exit(EXIT_FAILURE);
    }

    // setup everything socket related as well as the ctrl-c handler
    auto [sockfd, client_addr] = setup(std::stoi(argv[1]));

    // Bind to socket - necessary for receiver to get the data from the socket
    if (bind(sockfd, (const struct sockaddr *)&client_addr, sizeof(struct sockaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // start receiving data
    receive_loop(sockfd, (const struct sockaddr*)&client_addr);

    return 0;
}
