#ifndef ENGINE_RPM_HPP
#define ENGINE_RPM_HPP

#include "util/fh6_data.hpp"

class engine_rpm_t {
public:
    void init();
    void update(const fh6_data &);
    void close();
};


#endif