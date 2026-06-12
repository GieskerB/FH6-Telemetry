#ifndef SOCKET_SETUP_HPP
#define SOCKET_SETUP_HPP

#include <utility>

typedef std::pair<int,struct sockaddr_in> sock_info;

void bind_socket(int, const struct sockaddr*);
void receive_message(int, void*, const struct sockaddr*);

extern volatile bool running;

sock_info setup(int);

#endif
