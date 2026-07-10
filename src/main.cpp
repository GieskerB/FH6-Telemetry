#include <bits/stdc++.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_keycode.h>
#include <thread>
#include <array>
#include <semaphore>
#include <memory>
#include <variant>
#include <unistd.h>

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

#include "../include/udp/socket_setup.hpp"
#include "../include/util/texture_handler.hpp"
#include "../include/util/parse_args.hpp"

// Running variable to stop loop when program ends.
volatile bool running = true;

void render_thread(std::vector<telemetries_t>* telemetries, const std::array<unsigned short, 5>& sizes) {
    for (auto& telem : *telemetries) {
        std::visit([&](auto& obj) {
            if (sizes[obj.ID] != 0) obj.init(sizes[obj.ID]);
            else obj.init();
        },telem);
    }

    while(running) {
        for (auto& telem : *telemetries) {
            std::visit([](auto& obj) {
                obj.render();
            },telem);
        }
    }

    for (auto& telem : *telemetries) {
        std::visit([](auto& obj) {obj.close();},telem);
    }
}

// Continuously receive data via UDP.
void receive_loop(int sockfd, const struct sockaddr* client_addr, std::vector<telemetries_t>& telemetries, const std::array<unsigned short, 5>& sizes) {
    std::thread thread(render_thread, &telemetries, sizes);
    usleep(2000);

    struct fh6_data data_out;
    unsigned int last_time_stamp = 0;
    while (running) {
        // Call wrapper, exit if data could not be received.
        receive_message(sockfd, ((void*) &data_out), (const struct sockaddr*)&client_addr);

        if(last_time_stamp < data_out.TimestampMS) {
            last_time_stamp = data_out.TimestampMS;
        } else if (last_time_stamp > (std::numeric_limits<unsigned int>::max() - 1000) and data_out.TimestampMS < 1000) {
            last_time_stamp = data_out.TimestampMS;
        } else {
            continue;
        }

        for (auto& telem : telemetries) {
            std::visit([&](auto& obj) {obj.update(data_out);},telem);
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN and event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }
    }


    thread.join();

#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
}

int main(int argc, const char* argv[]) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if (!TTF_Init()) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }

    std::vector<telemetries_t> telemetries;
    std::array<unsigned short, 5> sizes;
    int port = parse_args(argc, argv, telemetries, sizes);

    // setup everything socket related
    auto [sockfd, client_addr] = setup(port);
    bind_socket(sockfd, (const struct sockaddr*)&client_addr);

    receive_loop(sockfd, (const struct sockaddr*)&client_addr,telemetries, sizes);

    destroy_registered_textures();

    SDL_Quit();
    TTF_Quit();

    return 0;
}
