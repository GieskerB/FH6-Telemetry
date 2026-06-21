#ifndef MAP_HPP
#define MAP_HPP

#include "util/fh6_data.hpp"

class map_t {
public:
    void init();
    void update(const fh6_data&);
    void close();
};

#endif