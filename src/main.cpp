#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <bits/stdc++.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <semaphore>
#include <thread>
#include <variant>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "../include/udp/socket_setup.hpp"
#include "../include/util/parse_args.hpp"
#include "../include/util/texture_handler.hpp"

// #define __TIMING__

// Running variable to stop loop when program ends.
volatile bool running = true;

std::binary_semaphore space_available(1);
std::binary_semaphore data_available(0);

void render_loop(std::vector<telemetry_variant_t>& telemetries) {
    while (running) {
        // Handle Events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN and event.key.key == SDLK_ESCAPE) {
                running = false;
            }
        }
        // Handle data
        data_available.acquire();
#ifdef __TIMING__
        std::vector<std::chrono::system_clock::time_point> timing;
        timing.push_back(std::chrono::system_clock::now());
#endif
        for (auto& telem : telemetries) {
            std::visit([&](auto& t) { t.render(); }, telem);
#ifdef __TIMING__
            timing.push_back(std::chrono::system_clock::now());
#endif
        }
#ifdef __TIMING__
        for (size_t i = 0; i < timing.size() - 1; ++i) {
            std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(timing[i + 1] - timing[i]).count()
                      << "ms ";
        }
        std::cout
            << "=> "
            << std::chrono::duration_cast<std::chrono::milliseconds>(timing[timing.size() - 1] - timing[0]).count()
            << "ms\n";
#endif
        space_available.release();
    }

    for (auto& telem : telemetries) {
        std::visit([](auto& obj) { obj.close(); }, telem);
    }
}

void data_loop(int port, std::vector<telemetry_variant_t>* telemetries) {
    auto [sockfd, client_addr] = setup(port);
    bind_socket(sockfd, (const struct sockaddr*)&client_addr);

    struct fh6_data data_out;
    unsigned int last_time_stamp = 0;
    unsigned long long frame = 0;
    while (running) {
        // Call wrapper, exit if data could not be received.
        receive_message(sockfd, ((void*)&data_out), (const struct sockaddr*)&client_addr);
        ++frame;

        if (last_time_stamp < data_out.TimestampMS) {
            last_time_stamp = data_out.TimestampMS;
        } else if (last_time_stamp > (std::numeric_limits<unsigned int>::max() - 1000) and
                   data_out.TimestampMS < 1000) {
            last_time_stamp = data_out.TimestampMS;
        } else {
            continue;
        }

        if (space_available.try_acquire()) {
            // Render was fast enough, ready to write next data.
            for (auto& telem : *telemetries) {
                std::visit([&](auto& obj) { obj.update(data_out); }, telem);
            }
            data_available.release();
        } else if (frame > 10) {
            // Render was too slow, skiping current data frame.
            std::cerr << "Render thread too slow, cant keep up - skipping frame <" << frame << ">!\n";
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
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if (!TTF_Init()) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }

    std::vector<telemetry_variant_t> telemetries;
    const int port = parse_args(argc, argv, telemetries);
    std::thread data_thread(data_loop, port, &telemetries);

    render_loop(telemetries);

    data_thread.join();
    destroy_registered_textures();

    SDL_Quit();
    TTF_Quit();
    return 0;
}
