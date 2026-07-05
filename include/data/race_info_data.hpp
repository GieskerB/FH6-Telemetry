#ifndef RACE_INFO_DATA
#define RACE_INFO_DATA

const char static_position_text[] = "Position:";
const char static_Lap_text[] = "Lap:";
const char static_total_race_time_text[] = "Total race time";
const char static_current_Lap_text[] = "Current Lap";
const char static_best_Lap_text[] = "Best Lap";
const char static_last_Lap_text[] = "Last Lap";
const char static_distance_text[] = "Distance (KM)";
const char static_shift_count_text[] = "Shift count";

struct race_info_data {
    bool new_data = false;
    char position[3]{0};
    char lap[3]{0};
    char total_time[12]{0};
    char current_lap[12]{0};
    char best_lap[12]{0};
    char last_lap[12]{0};
    char distance[8]{0};
    char shifts[5]{0};
};

#endif
