#ifndef ENGINE_RPM_HPP
#define ENGINE_RPM_HPP

#include <SDL3/SDL.h>

#include "../include/fh6_data.hpp"

namespace engine_rpm {

    void init();
    void update(const fh6_data &);
    void close();

}


#endif