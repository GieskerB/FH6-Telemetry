#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "../include/fh6_data.hpp"
#include "../include/socket_setup.hpp"

#include "../include/engine_rpm.hpp"

// Running variable to stop loop when program ends.
volatile bool running = true;

// Small wrapper around recvfrom
ssize_t receive_message(int sockfd, void* message, const struct sockaddr* client_addr) {
    static socklen_t len = sizeof(struct sockaddr);
    return recvfrom(sockfd, message, TELEMETRY_SIZE, MSG_WAITALL, (struct sockaddr *)&client_addr, &len);
}

// Continuously receive data via UDP.
void receive_loop(int sockfd, const struct sockaddr* client_addr) {
    unsigned int last_time_stamp = 0;
    while (running) {
        struct fh6_data data_out;

        // Call wrapper, exit if data could not be received.
        if (receive_message(sockfd, ((void*) &data_out), (const struct sockaddr*)&client_addr) < 0) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }

        if(last_time_stamp < data_out.TimestampMS) {
            last_time_stamp = data_out.TimestampMS;
        } else if (last_time_stamp > (std::numeric_limits<unsigned int>::max() - 1000) and data_out.TimestampMS < 1000) {
            last_time_stamp = data_out.TimestampMS;
        } else {
            continue;
        }

        engine_rpm::update(data_out);

        SDL_Event event;
        SDL_PollEvent(&event);
    }
    close(sockfd);
}
int main(int argc, const char* argv[]) {

    if(argc != 2) {
        perror("UPD test server requires one argument:\n\tmessage port\n");
        exit(EXIT_FAILURE);
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if (!TTF_Init()) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    engine_rpm::init();

    // setup everything socket related as well as the ctrl-c handler
    auto [sockfd, client_addr] = setup(std::stoi(argv[1]));

    // Bind to socket - necessary for receiver to get the data from the socket
    if (bind(sockfd, (const struct sockaddr *)&client_addr, sizeof(struct sockaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // start receiving data
    receive_loop(sockfd, (const struct sockaddr*)&client_addr);

    engine_rpm::close();

    SDL_Quit();
    TTF_Quit();

    return 0;
}
