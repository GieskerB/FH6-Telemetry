#ifndef PARSE_ARGS_HPP
#define PARSE_ARGS_HPP

#include <iostream>
#include <vector>
#include <variant>

#include "../engine_rpm.hpp"
#include "../gforce.hpp"
#include "../map.hpp"
#include "../car_info.hpp"

const char * TELEMETRIES[] = {"car-info", "engine-rpm", "g-force", "map"};
constexpr unsigned char TELEMETRY_COUNT = sizeof(TELEMETRIES) / sizeof(char *);

using telemetries_t = std::variant<car_info_t, engine_rpm_t, gforce_t, map_t>;

void print_help() {
    std::cout << "\n=====================================================================\n";
    std::cout << "                      FORZA HORIZON 6 TELEMETRY\n";
    std::cout << "=====================================================================\n\n";

    std::cout << "USAGE:\n";
    std::cout << "  fh6_telemetry -p <port> [options]\n\n";

    std::cout << "REQUIRED ARGUMENTS:\n";
    std::cout << "  -p, --port <number>      Specify the incoming UDP port (Range: 1024 - 65535).\n\n";

    std::cout << "OPTIONS:\n";
    std::cout << "  -h, --help               Display this help text and exit.\n";
    std::cout << "  -a, --all                Initialize and display all available telemetry windows.\n";
    std::cout << "                           Cannot be combined with individual -t selections.\n";
    std::cout << "  -t, --telemetry <name>   Enable a specific telemetry. You can specify a custom\n";
    std::cout << "                           window size using a colon suffix (:size).\n";
    std::cout << "                           Example: -t engine_rpm:500\n\n";

    std::cout << "AVAILABLE TELEMETRY TYPES  (Only one instance per type allowed):\n";
    
    for (unsigned char i = 0; i < TELEMETRY_COUNT; ++i) {
        std::cout << "  * " << TELEMETRIES[i] << "\n";
    }
    
    std::cout << "\nEXAMPLES:\n";
    std::cout << "  Listen on port 8000 with every telemtry enabled on default size:\n";
    std::cout << "    ./fh6_telemetry --port 8000 --all\n\n";
    std::cout << "  Listen on port 2345 with very engine-rpm telemetry:\n";
    std::cout << "    ./fh6_telemetry -p 2345 -t engine-rpm:1000\n";
    std::cout << "======================================================================\n" << std::endl;
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
        if(arg == "-a" or arg == "--all") {
            if (!push_unique(telemetries, car_info_t{})) need_help = true;
            if (!push_unique(telemetries, engine_rpm_t{})) need_help = true;
            if (!push_unique(telemetries, gforce_t{})) need_help = true;
            if (!push_unique(telemetries, map_t{})) need_help = true;
            if (need_help) {
                std::cerr << "[-a|--all] can only be used without [-t|--telemetry]!\n";
                break;
            }
            for (auto& telem: telemetries) {
                std::visit([](auto& obj) {obj.init();},telem);
            }
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


#endif