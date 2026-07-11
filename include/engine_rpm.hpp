#ifndef ENGINE_RPM_HPP
#define ENGINE_RPM_HPP

#include <memory>
#include <mutex>

#include "data/engine_rpm_data.hpp"
#include "data/fh6_data.hpp"

class engine_rpm_t {
    std::unique_ptr<std::mutex> mutex = std::make_unique<std::mutex>();
    engine_rpm_data data;

   public:
    const unsigned char ID = 1;
    void init(unsigned short = 400);
    void update(const fh6_data&);
    void render();
    void close();
};

#endif
