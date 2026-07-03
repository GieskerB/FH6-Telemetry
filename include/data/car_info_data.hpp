#ifndef CAR_INFO_DATA
#define CAR_INFO_DATA

constexpr unsigned char TEXT_WIDTH = 25;

struct car_info_data {
    bool new_data = false;
    char drive_train_path[23]{0};
    char class_id[3]{0};
    char performance_id[4]{0};
    char flag_path[29]{0};
    char group[TEXT_WIDTH]{0};
    char year_make[TEXT_WIDTH]{0};
    char model[TEXT_WIDTH]{0};
};

#endif
