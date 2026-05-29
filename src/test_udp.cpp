#include <bits/stdc++.h>
#include <arpa/inet.h>

#include "../include/fh6_data.hpp"
#include "../include/socket_setup.hpp"

// Running variable to stop loop when program ends.
volatile bool running = true;

// Small wrapper around sendto
ssize_t send_message(int sockfd, const void* message, const struct sockaddr* client_addr) {
    return sendto(sockfd, message, sizeof(struct fh6_data), 0, client_addr, sizeof(struct sockaddr));
}

// Continuously sends data via UDP.
void send_loop(int sockfd, const struct sockaddr* client_addr) {
    while (running) {

        // Dummy data for now...
        static struct fh6_data dummy_data;
        dummy_data.IsRaceOn++;

        // Call wrapper, exit if data could not be send.
        if (send_message(sockfd, ((const void*) &dummy_data), client_addr) < 0) {
            perror("sendto failed");
            exit(EXIT_FAILURE);;
        }

        // Sleep for a small duration.
        usleep(1000);
    }
    close(sockfd);
}

int main(int argc, const char* argv[]) {

    if(argc != 2) {
        perror("UPD test server requires one argument:\n\tMESSAGE port\n");
        exit(EXIT_FAILURE);
    }

    // setup everything socket related as well as the ctrl-c handler
    auto [sockfd, client_addr] = setup(std::stoi(argv[1]));

    // start sending data
    send_loop(sockfd, (const struct sockaddr*)&client_addr);
    return 0;
}
