#ifndef CAR_INFO_HPP
#define CAR_INFO_HPP

#include <memory>
#include <mutex>

#include "data/car_info_data.hpp"
#include "data/fh6_data.hpp"

class car_info_t {
    std::unique_ptr<std::mutex> mutex = std::make_unique<std::mutex>();
    car_info_data data;

   public:
    const unsigned char ID = 0;
    void init(unsigned short = 350);
    void update(const fh6_data&);
    void render();
    void close();
};

#endif
