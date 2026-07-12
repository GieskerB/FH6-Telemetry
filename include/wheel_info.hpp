#ifndef WHEEL_INFO_HPP
#define WHEEL_INFO_HPP

#include <memory>
#include <mutex>

#include "data/fh6_data.hpp"
#include "data/wheel_info_data.hpp"

class wheel_info_t {
    std::unique_ptr<std::mutex> mutex = std::make_unique<std::mutex>();
    wheel_info_data data;

   public:
    const unsigned char ID = 5;
    void init(unsigned short = 400);
    void update(const fh6_data&);
    void render();
    void close();
};

#endif
