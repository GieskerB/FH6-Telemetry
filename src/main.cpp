
#include <bits/stdc++.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

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


#include "../include/fh6_data.hpp"
#include "../include/socket_setup.hpp"
#include "../include/engine_rpm.hpp"

// Running variable to stop loop when program ends.
volatile bool running = true;

// Small wrapper around recvfrom
void receive_message(int sockfd, void* message, const struct sockaddr* client_addr) {
    static socklen_t len = sizeof(struct sockaddr);
    auto result = recvfrom(sockfd, static_cast<char*>(message), TELEMETRY_SIZE, MSG_WAITALL, (struct sockaddr *)&client_addr, &len);

#ifdef _WIN32
    if (result == SOCKET_ERROR) {
            std::cerr << "recvfrom failed. Windows Error: " << WSAGetLastError() << std::endl;
            exit(EXIT_FAILURE);
    }
#else
    if (result < 0) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
    }
#endif
}
// Small wrapper function around bind
void bind_socket(const int sockfd, const struct sockaddr * client_addr) {
        // Bind to socket - necessary for receiver to get the data from the socket
    auto result = bind(sockfd, client_addr, sizeof(struct sockaddr_in));
#ifdef _WIN32
    if (result == SOCKET_ERROR) {
        std::cerr << "Bind failed. Windows Error: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
#else
    if (result < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
#endif
}

// Continuously receive data via UDP.
void receive_loop(int sockfd, const struct sockaddr* client_addr) {
    unsigned int last_time_stamp = 0;
    while (running) {
        struct fh6_data data_out;

        // Call wrapper, exit if data could not be received.
        receive_message(sockfd, ((void*) &data_out), (const struct sockaddr*)&client_addr);

        if(last_time_stamp < data_out.TimestampMS) {
            last_time_stamp = data_out.TimestampMS;
        } else if (last_time_stamp > (std::numeric_limits<unsigned int>::max() - 1000) and data_out.TimestampMS < 1000) {
            last_time_stamp = data_out.TimestampMS;
        } else {
            continue;
        }

        engine_rpm::update(data_out);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_EventType::SDL_EVENT_QUIT) {
                running = false;
            }
        }
    }
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
}
int main(int argc, const char* argv[]) {

    if(argc != 2) {
        perror("FH6 telemetry requires one argument:\n\tmessage port\n");
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
    bind_socket(sockfd, (const struct sockaddr*)&client_addr);
    receive_loop(sockfd, (const struct sockaddr*)&client_addr);

    engine_rpm::close();

    SDL_Quit();
    TTF_Quit();

    return 0;
}
