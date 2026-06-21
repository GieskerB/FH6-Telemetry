
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
#include "../include/engine_rpm.hpp"
#include "../include/gforce.hpp"
#include "../include/map.hpp"
#include "../include/car_info.hpp"

using telemetries_t = std::variant<car_info_t, engine_rpm_t, gforce_t, map_t> ;

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
void receive_loop(int sockfd, const struct sockaddr* client_addr, std::vector<telemetries_t> telemetries) {
    unsigned int last_time_stamp = 0;

    std::vector<std::unique_ptr<thread_data_t>> thread_datas;
    for (auto& item : telemetries) {
        std::visit([&](auto& arg) {
            thread_datas.emplace_back(std::make_unique<thread_data_t>(
                nullptr, 
                [](void* inst, const fh6_data& d) {
                    static_cast<std::decay_t<decltype(arg)>*>(inst)->update(d);
                },
                &arg
            ));
        }, item);
    }

    std::vector<thread_data_t*> raw_pointers;
    for(const auto& item: thread_datas) {
        raw_pointers.push_back(item.get());
    }

    std::vector<std::thread> threads;
    for(size_t i = 0; i < telemetries.size(); ++i) {
        threads.emplace_back(run_thread, std::move(thread_datas[i]));
    }

    struct fh6_data data_out;
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

        for (const auto thread_data: raw_pointers) {
            thread_data->data = &data_out;
            thread_data->semaphore.release();
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // 1. Check if the user clicked the window's close button (highly recommended)
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            // 2. Check if a key was pressed down
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                // 3. Check if that specific key was the Escape key
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

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

    std::vector<telemetries_t> telemetries;

    telemetries.push_back(map_t{});
    telemetries.push_back(car_info_t{});
    telemetries.push_back(gforce_t{});
    telemetries.push_back(engine_rpm_t{});

    for (auto& telem : telemetries) {
        std::visit([](auto& obj) {obj.init();},telem);
    }

    // setup everything socket related as well as the ctrl-c handler
    auto [sockfd, client_addr] = setup(std::stoi(argv[1]));
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
