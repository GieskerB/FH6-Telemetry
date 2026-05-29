#ifndef SOCKET_SETUP_HPP
#define SOCKET_SETUP_HPP

#include <utility>

typedef std::pair<int,struct sockaddr_in> sock_info;

extern volatile bool running;

sock_info setup(int);

#endif
