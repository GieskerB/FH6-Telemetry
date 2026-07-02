#ifndef DATA_PER_FILE_HPP
#define DATA_PER_FILE_HPP

static const char* data_folder = "data_out";
constexpr unsigned int DATA_PER_FILE = 2048;

// Helper lambda to create name of file.
inline std::string make_filename(int folder_number, int file_num) {
    std::ostringstream ss;
    ss << data_folder << "/" << folder_number << "/" << DATA_PER_FILE << "-" << file_num << ".data_out";
    return ss.str();
};

#endif
