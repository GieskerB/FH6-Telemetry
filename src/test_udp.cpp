#include <bits/stdc++.h>
#include <arpa/inet.h>

#include "../include/fh6_data.hpp"
#include "../include/socket_setup.hpp"

enum test_mode {
    NONE,
    ENGINE
};

// Running variable to stop loop when program ends.
volatile bool running = true;

// Small wrapper around sendto
ssize_t send_message(int sockfd, const void* message, const struct sockaddr* client_addr) {
    return sendto(sockfd, message, sizeof(struct fh6_data), 0, client_addr, sizeof(struct sockaddr));
}

void set_data(fh6_data& data, test_mode tm) {
    switch (tm) {
    case ENGINE:
        data.EngineMaxRpm = 8500;
        data.EngineIdleRpm = 1650;
        if (data.CurrentEngineRpm == 0) data.CurrentEngineRpm = data.EngineIdleRpm;
        data.CurrentEngineRpm = data.CurrentEngineRpm * 1.0005;
        if(data.CurrentEngineRpm > data.EngineMaxRpm) {
            data.CurrentEngineRpm = data.EngineIdleRpm;
            if (++data.Gear > 11) data.Gear = 0;
        } 
        data.VelocityZ += 0.01;
        if(data.VelocityZ > 999 / 3.6) {
            data.VelocityZ = 0;
        }
        break;
    case NONE: 
        return;
    }
}

// Continuously sends data via UDP.
void send_loop(int sockfd, const struct sockaddr* client_addr, test_mode tm) {
    while (running) {

        // Dummy data for now...
        static struct fh6_data data_out;
        data_out.TimestampMS++;

        set_data(data_out,tm);

        // Call wrapper, exit if data could not be send.
        if (send_message(sockfd, ((const void*) &data_out), client_addr) < 0) {
            perror("sendto failed");
            exit(EXIT_FAILURE);;
        }

        // Sleep for a small duration.
        usleep(1000);
    }
    close(sockfd);
}

test_mode determine_test_mode (const char * test_mode_str) {
    if(strncmp("engine",test_mode_str,6) == 0) {
        return ENGINE;
    }

    return NONE;
}

int main(int argc, const char* argv[]) {

    if(argc < 2 or argc > 3) {
        perror("UPD test server requires one argument:\n\tMESSAGE port\n");
        exit(EXIT_FAILURE);
    }

    // setup everything socket related as well as the ctrl-c handler
    auto [sockfd, client_addr] = setup(std::stoi(argv[1]));

    test_mode tm = argc == 3 ? determine_test_mode(argv[2]) : NONE;

    // start sending data
    send_loop(sockfd, (const struct sockaddr*)&client_addr, tm);
    return 0;
}
