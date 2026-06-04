#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <filesystem>
#include <format>

#include "../include/socket_setup.hpp"
#include "../include/fh6_data.hpp"

static std::vector<fh6_data> data_vector;

static constexpr unsigned short DATA_PER_FILE = 2048;

static constexpr char* data_folder = "data_out";

volatile bool running = true;

void capture_loop(int sockfd, const struct sockaddr* client_addr) {
    std::filesystem::create_directory(data_folder);
    int file_counter = 1;
    while (running) {
        struct fh6_data data_out;

        // Call wrapper, exit if data could not be received.
        receive_message(sockfd, ((void*) &data_out), (const struct sockaddr*)&client_addr);

        data_vector.push_back(data_out);

        if (data_vector.size() >= DATA_PER_FILE) {
            std::ofstream output_file;
            output_file.open(std::format("{}/collected_data_({})_{}.data_out",data_folder,file_counter++,DATA_PER_FILE), std::ios::out|std::ios::binary);
            for(const auto & data: data_vector) {
                output_file.write((char*) &data, TELEMETRY_SIZE);
            }
            output_file.close();
            data_vector.clear();
            std::cout << "Wrote data" << (file_counter -1) << "\n";
        }
    }
    close(sockfd);
}

int main(int argc, char* argv[]) {

    if(argc != 2 ) {
        perror("UPD capture requires one argument:\n\tMESSAGE port\n");
        exit(EXIT_FAILURE);
    }

    auto [sockfd, client_addr] = setup(std::stoi(argv[1]));
    bind_socket(sockfd, (const struct sockaddr*)&client_addr);
    capture_loop(sockfd, (const struct sockaddr*)&client_addr);

    return 0;
}
