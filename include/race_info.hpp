#ifndef RACE_INFO_HPP
#define RACE_INFO_HPP

#include <mutex>
#include <memory>

#include "data/fh6_data.hpp"
#include "data/race_info_data.hpp"

class race_info_t {
    std::unique_ptr<std::mutex> mutex;
    race_info_data data;
public:
    void init(unsigned short  = 400);
    void update(const fh6_data&);
    void render();
    void close();
};

#endif
