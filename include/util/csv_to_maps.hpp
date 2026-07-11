#ifndef CSV_TO_MAPS_HPP
#define CSV_TO_MAPS_HPP

#include <string>
#include <unordered_map>

struct car_details_t {
    unsigned short id, year;
    std::string make, model;
};

std::unordered_map<int, car_details_t> car_details_map();
std::unordered_map<int, std::string> car_group_map();
std::unordered_map<std::string, std::string> car_country_map();

#endif
