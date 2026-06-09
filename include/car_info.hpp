#ifndef CAR_INFO_HPP
#define CAR_INFO_HPP

#include "../include/fh6_data.hpp"

namespace car_info {

    void init();
    void update(const fh6_data&);
    void close();

}

#endif