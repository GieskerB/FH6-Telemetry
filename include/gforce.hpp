#ifndef GFORCE_HPP
#define GFORCE_HPP

#include "../include/fh6_data.hpp"

namespace gforce {
    void init();
    void update(const fh6_data &);
    void close();
}

#endif
