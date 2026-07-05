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

struct thread_data_t {
    fh6_data* data;
    void (*update)(void* instance, const fh6_data&);
    void* instance_ptr;
    std::binary_semaphore semaphore{0};
};

void run_thread(std::unique_ptr<thread_data_t> thread_data) {
    while (running) {
        // Block until data is ready.
        thread_data->semaphore.acquire();
        thread_data->update(thread_data->instance_ptr, *thread_data->data);
    }
}

// Continuously receive data via UDP.
void receive_loop(int sockfd, const struct sockaddr* client_addr, std::vector<telemetries_t>& telemetries) {
    std::vector<thread_data_t*> raw_pointers;
    std::vector<std::thread> threads;
    for (auto& item : telemetries) {
        std::visit([&](auto& arg) {
            std::unique_ptr<thread_data_t> tmp = std::make_unique<thread_data_t>(
                nullptr,
                [](void* inst, const fh6_data& d) {
                    static_cast<std::decay_t<decltype(arg)>*>(inst)->update(d);
                    static_cast<std::decay_t<decltype(arg)>*>(inst)->render();
                },
                &arg
            );
            raw_pointers.push_back(tmp.get());
            threads.emplace_back(run_thread, std::move(tmp));
        }, item);
    }

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

        for (const auto& thread_data: raw_pointers) {
            thread_data->data = &data_out;
            thread_data->semaphore.release();
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

    for (const auto& thread_data: raw_pointers) {
        // One last release such that the threads can finish
        thread_data->semaphore.release();
    }

    for (auto& thread: threads) {
        thread.join();
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

    std::vector<telemetries_t> telemetries;
    int port = parse_args(argc, argv, telemetries);

    // setup everything socket related
    auto [sockfd, client_addr] = setup(port);
    bind_socket(sockfd, (const struct sockaddr*)&client_addr);

    receive_loop(sockfd, (const struct sockaddr*)&client_addr,telemetries);

    for (auto& telem : telemetries) {
        std::visit([](auto& obj) {obj.close();},telem);
    }

    destroy_registered_textures();

    SDL_Quit();
    TTF_Quit();

    return 0;
}
