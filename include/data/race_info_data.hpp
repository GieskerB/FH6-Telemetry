#ifndef RACE_INFO_DATA
#define RACE_INFO_DATA

const char static_position_text[] = "Position:";
const char static_lap_text[] = "Lap:";
const char static_race_time_text[] = "Total race time";
const char static_current_lap_text[] = "Current Lap";
const char static_best_lap_text[] = "Best Lap";
const char static_last_lap_text[] = "Last Lap";
const char static_distance_text[] = "Distance (KM)";
const char static_shifts_text[] = "Shift count";

struct race_info_data {
    bool is_paused = false;
    unsigned char new_data = 0;
    char position[3]{0};
    char lap[3]{0};
    char race_time[12]{0};
    char current_lap[12]{0};
    char best_lap[12]{0};
    char last_lap[12]{0};
    char distance[9]{0};
    char shifts[5]{0};
};

#endif
