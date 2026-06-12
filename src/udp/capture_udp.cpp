#include <bits/stdc++.h>
#include <filesystem>
#include <format>
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif

#include "../../include/udp/socket_setup.hpp"
#include "../../include/util/fh6_data.hpp"
#include "../../include/util/data_per_file.hpp"

static std::vector<fh6_data> data_vector;

volatile bool running = true;

void capture_loop(int sockfd, const struct sockaddr* client_addr) {
    std::filesystem::create_directory(data_folder);
    unsigned int file_counter = 0;
    while (running) {
        struct fh6_data data_out;

        // Call wrapper, exit if data could not be received.
        receive_message(sockfd, ((void*) &data_out), (const struct sockaddr*)&client_addr);

        data_vector.push_back(data_out);

        std::cout << "Pos: " << data_out.PositionX << " " << data_out.PositionZ<< '\n';

        if (data_vector.size() >= DATA_PER_FILE) {
            std::ofstream output_file;

            output_file.open(make_filename(file_counter++), std::ios::out|std::ios::binary);
            for(const auto & data: data_vector) {
                output_file.write((char*) &data, TELEMETRY_SIZE);
            }
            output_file.close();
            data_vector.clear();
        }
    }
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
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
