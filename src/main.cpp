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

const char * TELEMETRIES[] = {"car-info", "engine-rpm", "g-force", "map"};
constexpr unsigned char TELEMETRY_COUNT = sizeof(TELEMETRIES) / sizeof(char *);

using telemetries_t = std::variant<car_info_t, engine_rpm_t, gforce_t, map_t>;

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
    std::vector<thread_data_t*> raw_pointers;
    std::vector<std::thread> threads;
    for (auto& item : telemetries) {
        std::visit([&](auto& arg) {
            std::unique_ptr<thread_data_t> tmp = std::make_unique<thread_data_t>(
                nullptr, 
                [](void* inst, const fh6_data& d) {
                    static_cast<std::decay_t<decltype(arg)>*>(inst)->update(d);
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

void print_help() {
    std::cout << "\n";
}

bool push_unique(std::vector<telemetries_t>& vec, const telemetries_t& value) {
    // Look through the vector to see if any element has the SAME type index
    auto it = std::find_if(vec.begin(), vec.end(), [index_to_match = value.index()](const telemetries_t& v) {
        return v.index() == index_to_match;
    });

    // If no matching type index was found, push it!
    if (it == vec.end()) {
        vec.push_back(value);
        return true; 
    }

    return false; 
}

bool handle_telemetry_arg(std::string arg, std::vector<telemetries_t>& telemetries) {
    const size_t colon_pos = arg.find(':');
    const bool specify_size = std::string::npos != colon_pos;

    const std::string& telemetry_name = specify_size ? arg.substr(0,colon_pos) : arg;

    for(unsigned char i = 0; i < TELEMETRY_COUNT; ++i) {
        if(telemetry_name == TELEMETRIES[i]) {
            telemetries_t telem;

            switch (i){
            case 0:
                telem = car_info_t{};
                break;
            case 1:
                telem = engine_rpm_t{};
                break;
            case 2:
                telem = gforce_t{};
                break;
            case 3:
                telem = map_t{};
                break;
            default:
                break;
            }
            
            if(specify_size) {
                const size_t number_start = colon_pos+1;
                if(number_start >= arg.size()) {
                    std::cerr << "Number required after colon size specifier!\n";
                    return true;
                }
                std::string tmp;
                try {
                    tmp = arg.substr(number_start, arg.size()-number_start);
                    const int size = std::stoi(tmp);
                    const unsigned short max_value = std::numeric_limits<unsigned short>::max();
                    if(size < 0 or size > max_value) {
                        std::cerr << "Size of telemetry must be in range [0, "<< max_value <<"]!\n";
                        return true;
                    }
                    std::visit([size](auto& obj) {obj.init(static_cast<unsigned short>(size));},telem);
                }  catch (std::invalid_argument&) {
                    std::cerr << "Can not parse " << tmp << " into a size for telemetry number!\n";
                    return true;
                }
            } else {
                std::visit([](auto& obj) {obj.init();},telem);
            }

            if (!push_unique(telemetries,telem)) {
                std::cerr << "Cannot instanace same telemetry more then once!\n";
                return true;
            }

            return false;
        }
    }
    std::cerr << "'" << telemetry_name << "' is not part of the implemented telemetry set!\n";
    return true;
}

int parse_args(int argc, const char* argv[], std::vector<telemetries_t>& telemetries) {
    bool has_port_input = false;
    bool need_help = false;
    int port_number = 0;
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
            need_help = handle_telemetry_arg(argv[++i], telemetries);
            if(need_help) break;
            continue;
        }
        std::cerr << "Unknown argument '" << arg << "'!\n";
        need_help = true;
        break;
    }

    if (!has_port_input and !need_help) {
        std::cerr << "Port number is required for this program to run!\n";
        need_help = true;
    }

    if(need_help) {
        print_help();
        exit(EXIT_SUCCESS);
    }
    return port_number;
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

    // setup everything socket related as well as the ctrl-c handler
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
