#include <bits/stdc++.h>
#include <arpa/inet.h>

#include "../../include/util/fh6_data.hpp"
#include "../../include/udp/socket_setup.hpp"
#include "../../include/util/data_per_file.hpp"

// Running variable to stop loop when program ends.
volatile bool running = true;

// Small wrapper around sendto
ssize_t send_message(int sockfd, const void* message, const struct sockaddr* client_addr) {
    return sendto(sockfd, message, sizeof(struct fh6_data), 0, client_addr, sizeof(struct sockaddr));
}

void get_data(fh6_data& data, int folder_number) {

    static unsigned int current_file = 0;
    static unsigned int current_blob = 0;
    static unsigned int timestamp_override = 0;
    static std::ifstream file;

    // Open file
    if (!file.is_open()) {

        file.open(make_filename(folder_number, current_file), std::ios::binary);

        // If failed, it just wraps around.
        if (!file) {
            current_file = 0;
            file.open(make_filename(folder_number, current_file), std::ios::binary);

            // If wrap around does not work. exit with error.
            if (!file) {
                // Replaced perror with std::cerr to easily print the std::string
                std::cerr << "Could not open file '" << make_filename(folder_number, current_file) << "': " << strerror(errno) << "\n";
                exit(EXIT_FAILURE);
            }
        }

        current_blob = 0;
    }

    // read from file.
    file.read(reinterpret_cast<char*>(&data), TELEMETRY_SIZE);
    current_blob++;

    if (!file) {
        perror("Could not read data from file");
        file.close();
        exit(EXIT_FAILURE);
    }

    // once all blobs form a file has been read, close it and move to the next!
    if (current_blob >= DATA_PER_FILE) {
        file.close();
        current_file++;
    }

    // fh6_telemetry drops udp packages when not in order. Override Timestamp to prevent this.
    data.TimestampMS = timestamp_override++;
    if(timestamp_override == std::numeric_limits<unsigned int>::max()) {
        timestamp_override = 0;
    }
}

// Continuously sends data via UDP.
void send_loop(int sockfd, const struct sockaddr* client_addr, int folder_number, int delay) {
    while (running) {
        static struct fh6_data data_out;

        get_data(data_out, folder_number);

        // Call wrapper, exit if data could not be send.
        if (send_message(sockfd, ((const void*) &data_out), client_addr) < 0) {
            perror("sendto failed");
            exit(EXIT_FAILURE);;
        }
        // Sleep for small duration to roughly match 60 Hz.
        usleep(delay);
    }
    close(sockfd);
}

bool run_recorded(const char * test_mode_str) {
    return strncmp("recorded",test_mode_str,9) == 0;
}

int main(int argc, const char* argv[]) {

    if(argc != 4) {
        perror("UPD test server requires one argument:\n\tMESSAGE PORT\n\tFOLDER NUMBER\n\t DELAY AFTER SEND (ms)\n");
        exit(EXIT_FAILURE);
    }

    // setup everything socket related as well as the ctrl-c handler
    auto [sockfd, client_addr] = setup(std::stoi(argv[1]));

    // start sending data
    send_loop(sockfd, (const struct sockaddr*)&client_addr, std::stoi(argv[2]),std::stoi(argv[3]));
    return 0;
}
