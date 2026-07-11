#ifndef GFORCE_HPP
#define GFORCE_HPP

#include <memory>
#include <mutex>

#include "data/fh6_data.hpp"
#include "data/gforce_data.hpp"

class gforce_t {
    std::unique_ptr<std::mutex> mutex = std::make_unique<std::mutex>();
    gforce_data data;

   public:
    const unsigned char ID = 2;
    void init(unsigned short = 300);
    void update(const fh6_data&);
    void render();
    void close();
};

#endif
