#ifndef ENGINE_RPM_HPP
#define ENGINE_RPM_HPP

#include <mutex>
#include <memory>

#include "data/fh6_data.hpp"
#include "data/engine_rpm_data.hpp"

class engine_rpm_t {
    std::unique_ptr<std::mutex> mutex;
    engine_rpm_data data;
public:
    void init(unsigned short  = 400);
    void update(const fh6_data &);
    void render();
    void close();
};


#endif
