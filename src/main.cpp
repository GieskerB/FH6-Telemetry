#include <bits/stdc++.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_keycode.h>
#include <thread>
#include <array>
#include <semaphore>
#include <memory>

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

#include "../include/engine_rpm.hpp"
#include "../include/gforce.hpp"
#include "../include/map.hpp"
#include "../include/car_info.hpp"
#include "../include/udp/socket_setup.hpp"
#include "../include/util/texture_handler.hpp"

const char * TELEMETRIES[] = {"car-info", "engine-rpm", "g-force", "map"};
constexpr unsigned short DEFAULT_SIZES[] = {100,200,300,400};
constexpr unsigned char TELEMETRY_COUNT = sizeof(TELEMETRIES) / sizeof(char *);

// Running variable to stop loop when program ends.
volatile bool running = true;

struct arg_data_t {
    unsigned short port{0};
    bool show_telemetry[TELEMETRY_COUNT]{false};
    unsigned short telemetry_size[TELEMETRY_COUNT]{0};
};

struct thread_data_t {
    fh6_data* data;
    void (*update) (const fh6_data&);
    std::binary_semaphore semaphore{0};
};

void run_thread(thread_data_t* thread_data) {
    while (running) {
        // Block until data is ready.
        thread_data->semaphore.acquire();
        const fh6_data& f_data = *thread_data->data;
        thread_data->update(f_data);
    }
}

// Continuously receive data via UDP.
void receive_loop(int sockfd, const struct sockaddr* client_addr, const arg_data_t& arg_data) {
    unsigned int last_time_stamp = 0;

    std::vector<std::unique_ptr<thread_data_t>> thread_data_vector {
        // thread_data_t{std::binary_semaphore{0}, nullptr, engine_rpm::update},
        // thread_data_t{std::binary_semaphore{0}, nullptr, gforce::update},
        // thread_data_t{std::binary_semaphore{0}, nullptr, map::update},
        // thread_data_t{std::binary_semaphore{0}, nullptr, car_info::update},
    };

    for(int i= 0; i < TELEMETRY_COUNT; ++i) {
        if(arg_data.show_telemetry[i]) {
            switch (i) {
            case 0:
                thread_data_vector.push_back(std::make_unique<thread_data_t>( nullptr, car_info::update));
                break;
            case 1:
                thread_data_vector.push_back(std::make_unique<thread_data_t>( nullptr, engine_rpm::update));
                break;
            case 2:
                thread_data_vector.push_back(std::make_unique<thread_data_t>( nullptr, gforce::update));
                break;
            case 3:
                thread_data_vector.push_back(std::make_unique<thread_data_t>( nullptr, map::update));
                break;
            }
        }
    }

    std::vector<std::thread> thread_vector;
    for(size_t i = 0; i < thread_data_vector.size(); ++i) {
        thread_vector.emplace_back(run_thread, &*thread_data_vector[i]);
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

        for (size_t i = 0 ; i < thread_data_vector.size(); ++i) {
            thread_data_vector[i]->data = &data_out;
            thread_data_vector[i]->semaphore.release();
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

    for (size_t i = 0 ; i < thread_data_vector.size(); ++i) {
        thread_vector[i].join();
    }

#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
}

void print_help() {
    std::cout << "\n";
}

bool handle_telemetry_arg(std::string arg, arg_data_t& arg_data, bool& need_help) {
    const size_t colon_pos = arg.find(':');
    const bool specify_size = std::string::npos != colon_pos;

    const std::string& telemetry_name = specify_size ? arg.substr(0,colon_pos) : arg;

    bool found_telemetry = false;

    std::cout << "TELEM name" << telemetry_name << "\n";

    for(unsigned char i = 0; i < TELEMETRY_COUNT; ++i) {
        if(telemetry_name == TELEMETRIES[i]) {
            arg_data.show_telemetry[i] = true;
            if(specify_size) {
                const size_t number_start = colon_pos+1;
                if(number_start <= arg.size()) {
                    std::cerr << "Number required after colon size specifier!\n";
                    need_help = true;
                    return false;
                }
                std::string tmp;
                try {
                    tmp = arg.substr(number_start, arg.size()-number_start);
                    const int size = std::stoi(tmp);
                    const unsigned short max_value = std::numeric_limits<unsigned short>::max();
                    if(size < 0 or size > max_value) {
                        std::cerr << "Size of telemetry must be in range [0, "<< max_value <<"]!\n";
                        need_help = true;
                        return false;
                    }
                    arg_data.telemetry_size[i] = static_cast<unsigned short>(size);
                }  catch (std::invalid_argument&) {
                    std::cerr << "Can not parse " << tmp << " into a size for telemetry number!\n";
                    need_help = true;
                    return false;
                }
            } else {
                arg_data.telemetry_size[i] = DEFAULT_SIZES[i];
            }
        }
    }

    return found_telemetry;
}

arg_data_t parse_args(int argc, const char* argv[]) {
    bool has_port_input = false;
    bool need_help = false;
    arg_data_t arg_data{};
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        // simple things first! Help:
        if(arg == "-h" or arg == "--help") {
            need_help = true; 
            break;
        }
        if(arg == "-p" or arg == "--port") {
            if(i + 1 >= argc) {
                std::cerr << "Missing argument after [-p|--port]!\nRequires port number as an argument.\n";
                need_help = true;
                break;
            }
            int port_number = 0;
            try {
                port_number = std::stoi(argv[++i]);
            } catch (const std::invalid_argument&) {
                std::cerr << "Can not parse " << argv[i] << " into a port number!\n";
                need_help = true;
                break;
            }
            if(port_number < 1024 or port_number > 65535) {
                std::cerr << "Port must be between 1024 and 65535!\n";
                need_help = true;
                break;
            }

            has_port_input = true;
            continue;
        }
        if(arg == "-t" or arg == "--telemetry") {
            if(i + 1 >= argc) {
                std::cerr << "Missing argument after [-t|--telemetry]!\nRequires name of telemetry window as an argument.\n";
                need_help = true;
                break;
            }
            const bool found = handle_telemetry_arg(argv[++i], arg_data, need_help);
            if(!found and need_help) break;
            if(found) std::cout << argv[i] << "\n";
            if(found) continue;
        }
    }

    if (!has_port_input and !need_help) {
        std::cerr << "Port number is required for this program to run!\n";
        need_help = true;
    }

    if(need_help) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    return arg_data;
}

int main(int argc, const char* argv[]) {

    const arg_data_t arg_data = parse_args(argc,argv);

    // if(argc != 2) {
    //     perror("FH6 telemetry requires one argument:\n\tmessage port\n");
    //     exit(EXIT_FAILURE);
    // }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if (!TTF_Init()) {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int thread_count = 0;
    for(int i= 0; i < TELEMETRY_COUNT; ++i) {
        if(arg_data.show_telemetry[i]) {
            ++thread_count;
            switch (i)
            {
            case 0:
                map::init();
                break;
            case 1:
                engine_rpm::init();
                break;
            case 2:
                gforce::init();
                break;
            case 3:
                car_info::init();
                break;
            }        
        }
    }

    // setup everything socket related as well as the ctrl-c handler
    auto [sockfd, client_addr] = setup(arg_data.port);
    bind_socket(sockfd, (const struct sockaddr*)&client_addr);
    receive_loop(sockfd, (const struct sockaddr*)&client_addr, arg_data);

    for(int i= 0; i < TELEMETRY_COUNT; ++i) {
        if(arg_data.show_telemetry[i]) {
            switch (i)
            {
            case 0:
                map::close();
                break;
            case 1:
                engine_rpm::close();
                break;
            case 2:
                gforce::close();
                break;
            case 3:
                car_info::close();
                break;
            }        
        }
    }

    destroy_registered_textures();

    SDL_Quit();
    TTF_Quit();

    return 0;
}
