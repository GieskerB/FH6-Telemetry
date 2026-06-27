#include <fstream>
#include <sstream>
#include <iostream>

#include "../../include/util/csv_to_maps.hpp"

std::unordered_map<int, car_details_t> car_details_map() {
    std::ifstream csv_file {"csv/fh6_car_database.csv"};
    if (!csv_file.is_open()) {
        std::cerr << "Could not open csv/fh6_car_database.csv!\n";
        exit(EXIT_FAILURE);
    }

    std::unordered_map<int, car_details_t> car_map;
    
    std::string line;
    std::getline(csv_file,line); // skip first line (csv header)
    while (std::getline(csv_file,line,'\n')) {
    
        std::stringstream str_stream(line);

        std::string id_str, year_str, __discard;
        car_details_t details;
        
        std::getline(str_stream, id_str, ',');
        std::getline(str_stream, year_str, ',');
        std::getline(str_stream, details.make, ',');
        std::getline(str_stream, details.model, ',');
        std::getline(str_stream, __discard,'\r');

        // Omit error handling...
        int id = stoi(id_str);
        details.year = stoi(year_str);

        car_map.insert(std::pair<int,car_details_t> (id, details));
    }
    return car_map;
}

std::unordered_map<int, std::string> car_group_map() {
    std::ifstream csv_file {"csv/fh6_group_database.csv"};
    if (!csv_file.is_open()) {
        std::cerr << "Could not open csv/fh6_group_database.csv!\n";
        exit(EXIT_FAILURE);
    }

    std::unordered_map<int, std::string> group_map;
    
    std::string line;
    std::getline(csv_file,line); // skip first line (csv header)
    while (std::getline(csv_file,line,'\n')) {
    
        std::stringstream str_stream(line);

        std::string id_str, group_str;
        
        std::getline(str_stream, id_str, ',');
        std::getline(str_stream, group_str, '\r');


        // Omit error handling...
        int id = stoi(id_str);

        group_map.insert(std::pair<int,std::string> (id, group_str));
    }
    return group_map;
}

std::unordered_map<std::string, std::string> car_make_map() {
        std::ifstream csv_file {"csv/fh6_make_database.csv"};
    if (!csv_file.is_open()) {
        std::cerr << "Could not open csv/fh6_make_database.csv!\n";
        exit(EXIT_FAILURE);
    }

    std::unordered_map<std::string, std::string> make_map;
    
    std::string line;
    std::getline(csv_file,line); // skip first line (csv header)
    while (std::getline(csv_file,line,'\n')) {
    
        std::stringstream str_stream(line);

        std::string make_str, country_str;
        
        std::getline(str_stream, make_str, ',');
        std::getline(str_stream, country_str, '\r');



        make_map.insert(std::pair<std::string,std::string> (make_str, country_str));
    }
    return make_map;
}
