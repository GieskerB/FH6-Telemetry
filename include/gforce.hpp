#ifndef GFORCE_HPP
#define GFORCE_HPP

#include "util/fh6_data.hpp"

class gforce_t {
public:
    void init();
    void update(const fh6_data &);
    void close();
};

#endif
