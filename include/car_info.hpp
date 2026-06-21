#ifndef CAR_INFO_HPP
#define CAR_INFO_HPP

#include "util/fh6_data.hpp"

class car_info_t {
public:
    void init();
    void update(const fh6_data&);
    void close();
};

#endif