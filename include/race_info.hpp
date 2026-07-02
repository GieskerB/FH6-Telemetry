#ifndef RACE_INFO_HPP
#define RACE_INFO_HPP

#include "util/fh6_data.hpp"

class race_info_t {
public:
    void init(unsigned short  = 400);
    void update(const fh6_data&);
    void close();
};

#endif