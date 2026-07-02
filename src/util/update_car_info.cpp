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

static int get_id(const std::string& car_folder, const std::string& zip_filename) {
    std::filesystem::path zip_path = std::filesystem::path(car_folder) / zip_filename;
    std::filesystem::path temp_dir = std::filesystem::current_path() / "tmp_fh6_zip"; 
   
    int id = -1;
    try {
        std::filesystem::create_directories(temp_dir);

        // Build the OS-specific command string
        std::string command;
#if defined(_WIN32)
        // Windows: Use PowerShell's built-in Expand-Archive
        command = "powershell -Command \"Expand-Archive -Path '" + zip_path.string() + 
                  "' -DestinationPath '" + temp_dir.string() + "' -Force\"";
#else
        // Linux / macOS: Use the standard 'unzip' utility
        command = "unzip -q \"" + zip_path.string() + "\" -d \"" + temp_dir.string() + "\"";
#endif

        // Execute the command        
        if (std::system(command.c_str()) != 0) {
            std::cerr << "Extraction failed !\n" << std::endl;
            std::filesystem::remove_all(temp_dir);
            return -1;
        }

        std::filesystem::path id_file_directory = std::filesystem::path(temp_dir) / "Scene/animations/Mojo/clip";

        // Should only be one!
        for (const auto & entry : std::filesystem::directory_iterator(id_file_directory)) {
            const std::string id_filename = entry.path().filename();
            const char* digits = "0123456789";
            const std::size_t  n = id_filename.find_first_of(digits);
            if (n != std::string::npos) {
                std::size_t const m = id_filename.find_first_not_of(digits, n);
                id = std::stoi(id_filename.substr(n, m != std::string::npos ? m-n : m));
            };
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred during extraction: " << e.what() << std::endl;
    }

    // Delete the temporary directory and all its contents
    try {
        std::filesystem::remove_all(temp_dir);
        return id;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Cleanup failed: " << e.what() << std::endl;
    }
    return -1;
}

static void add_car(const std::string& car_folder , const std::string& filename) {
    int year;
    std::string make, model;
    std::cout << "FOUND NEW CAR [" << filename << "]!!!\n"
                << "Following informations are need to add it to the database (Year, Make, Model)!\n"
                << "===Year:  ";
    std::cin >> year;
    std::cout << "===Make:  ";
    std::cin >> make;
    std::cout << "===Model: ";
    std::cin >> model;

    int id = get_id(car_folder, filename);

    std::ofstream csv_database;
    csv_database.open("csv/fh6_car_database.csv", std::ios::app);
    csv_database << id << ',' << year << ',' << make << ',' << model << ',' << filename << '\n';
    csv_database.close();

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
            if(!set.contains(filename) and filename.find("_Traffic_") == std::string::npos) {
                add_car(path, filename);
            }
        }
    }
        
    return 0;
}