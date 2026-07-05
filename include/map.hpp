#ifndef MAP_HPP
#define MAP_HPP

#include <mutex>
#include <memory>

#include "data/fh6_data.hpp"
#include "data/map_data.hpp"

class map_t {
    std::unique_ptr<std::mutex> mutex;
    map_data data;
public:
    void init(unsigned short  = 780);
    void update(const fh6_data&);
    void render();
    void close();
};

#endif
