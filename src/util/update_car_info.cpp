#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <filesystem>
#include <algorithm>

std::set<std::string> get_zip_set() {
    std::ifstream csv_file {"csv/fh6_car_database.csv"};
    if (!csv_file.is_open()) {
        std::cerr << "Could not open CSV file!\n";
        exit(EXIT_FAILURE);
    }

    std::set<std::string> zip_set;
    
    std::string line;
    std::getline(csv_file,line); // skip first line (csv header)
    while (std::getline(csv_file,line,'\n')) {
    
        std::stringstream str_stream(line);

        std::string zip;
        
        std::getline(str_stream, zip, ','); // read id & discard
        std::getline(str_stream, zip, ','); // read year & discard
        std::getline(str_stream, zip, ','); // read make & discard
        std::getline(str_stream, zip, ','); // read model & discard
        std::getline(str_stream, zip,'\r'); // read zip name

        zip_set.insert(zip);
    }
    return zip_set;
}


int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Need path to Cars directory of FH6 installation!\n";
        exit(EXIT_FAILURE);
    }
    const auto set = get_zip_set();

    const std::string path = argv[1];
    for (const auto & entry : std::filesystem::directory_iterator(path)){
        const std::string filename = entry.path().filename().string();
        const bool is_zip = filename.substr(filename.length() -4, 4) == ".zip";
        if (is_zip) {
            if(!set.contains(filename) and filename.find("_Traffic_") == std::string::npos)  {
                std::cout << "FOUND NEW CAR [" << filename << "]!!!\nDon't forget to add it to the database!";
            }
        }
    }
        
    return 0;
}