
#include <bits/stdc++.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_keycode.h>
#include <thread>
#include <array>
#include <semaphore>

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

#include "include/engine_rpm.hpp"
#include "include/gforce.hpp"
#include "include/map.hpp"
#include "include/car_info.hpp"
#include "include/udp/socket_setup.hpp"
#include "include/util/texture_handler.hpp"

constexpr unsigned char THREAD_COUNT = 4;

// Running variable to stop loop when program ends.
volatile bool running = true;

struct thread_data {
    std::binary_semaphore semaphore;
    fh6_data* data;
    void (*update) (const fh6_data&);
};

void run_thread(thread_data* t_data) {
    while (running) {
        // Block until data is ready.
        t_data->semaphore.acquire();
        const fh6_data& f_data = *t_data->data;
        std::cout << "TS: " << f_data.TimestampMS<<"\n";
        t_data->update(f_data);
    }
}

// Continuously receive data via UDP.
void receive_loop(int sockfd, const struct sockaddr* client_addr) {
    unsigned int last_time_stamp = 0;

    std::array<thread_data,THREAD_COUNT> thread_data_array {
        thread_data{std::binary_semaphore{0}, nullptr, engine_rpm::update},
        thread_data{std::binary_semaphore{0}, nullptr, gforce::update},
        thread_data{std::binary_semaphore{0}, nullptr, map::update},
        thread_data{std::binary_semaphore{0}, nullptr, car_info::update},
    };

    std::vector<std::thread> thread_vector;
    for(int i = 0; i < THREAD_COUNT; ++i) {
        thread_vector.emplace_back(run_thread, &thread_data_array[i]);
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

        for (int i = 0 ; i < THREAD_COUNT; ++i) {
            thread_data_array[i].data = &data_out;
            thread_data_array[i].semaphore.release();
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

    for (int i = 0 ; i < THREAD_COUNT; ++i) {
        thread_vector[i].join();
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
    gforce::init();
    map::init();
    car_info::init();

    // setup everything socket related as well as the ctrl-c handler
    auto [sockfd, client_addr] = setup(std::stoi(argv[1]));
    bind_socket(sockfd, (const struct sockaddr*)&client_addr);
    receive_loop(sockfd, (const struct sockaddr*)&client_addr);

    engine_rpm::close();
    gforce::close();
    map::close();
    car_info::close();

    destroy_registered_textures();

    SDL_Quit();
    TTF_Quit();

    return 0;
}
