#ifndef GFORCE_HPP
#define GFORCE_HPP

#include <SDL3/SDL.h>

#include "../include/fh6_data.hpp"

namespace gforce {

    void init();
    void update(const fh6_data &);
    void close();

}

#endif
