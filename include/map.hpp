#ifndef MAP_HPP
#define MAP_HPP

#include "util/fh6_data.hpp"

namespace map {
    void init();
    void update(const fh6_data&);
    void close();
}

#endif