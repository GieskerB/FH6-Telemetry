#ifndef TYRES_HPP
#define TYReS_HPP

#include "fh6_data.hpp"

namespace tires {

    void init();
    void update(const fh6_data&);
    void close();

}

#endif