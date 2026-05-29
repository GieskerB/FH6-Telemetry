#include <bits/stdc++.h>
#include <arpa/inet.h>

#include "../include/fh6_data.hpp"

// Running variable to stop loop when program ends.
static volatile bool running = true;

// Initialize UDP socket for test server
int create_socket(int server_port) {
    int server_socket;

    struct sockaddr_in servaddr;

    // Clear memory of struct for safety reasons.
    memset(&servaddr, 0, sizeof(servaddr));

    // Creating socket file descriptor
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("failed to create socket for UDP test server.\n");
        exit(EXIT_FAILURE);
    }

    // set addres information to IPv4 and designated port number
    servaddr.sin_family    = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(server_port);

    // Finally bin the socket to that specific address.
    if (bind(server_socket, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("failed to bind socket of UDP test server.\n");
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

// Small wrapper around sendto
ssize_t send_message(int server_socket, const void* message, const struct sockaddr* client_addr) {
    return sendto(server_socket, message, sizeof(message), 0, client_addr, sizeof(struct sockaddr));
}

// Continuously sends data via UDP.
void send_loop(int server_socket, const struct sockaddr* client_addr) {
    while (running) {

        // Dummy data for now...
        struct fh6_data dummy_data;

        // Call wrapper, exit if data could not be send.
        if (send_message(server_socket, ((const char*) &dummy_data), client_addr) < 0) {
            perror("Sendto failed");
            exit(EXIT_FAILURE);;
        }

        // Sleep for a small duration.
        usleep(1000);
    }
}

// Correctly sets up the client adress in local host with provided port number.
void setup_client_address(struct sockaddr_in& client_addr, int client_port) {
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr);
}

// When program is ended with ctrl-c the socket gets closed and the loop ended.
void handle(int server_socket) {
    running = false;
    close(server_socket);
}

int main(int argc, const char* argv[]) {

    if(argc != 3) {
        perror("UPD test server requires two arguments:\n\tSERVER port,\n\tCLIENT port\n");
        exit(EXIT_FAILURE);
    }

    // setup server
    const int server_socket = create_socket(std::stoi(argv[1]));

    // setup handle function
    struct sigaction ctrl_c_handler;
    ctrl_c_handler.sa_handler = handle;
    sigemptyset(&ctrl_c_handler.sa_mask);
    ctrl_c_handler.sa_flags = server_socket;
    sigaction(SIGINT, &ctrl_c_handler, NULL);

    // setup client connection
    struct sockaddr_in client_addr;
    setup_client_address(client_addr, std::stoi(argv[2]));

    // start sending data.
    send_loop(server_socket, (const struct sockaddr*)&client_addr);

    return 0;
}
