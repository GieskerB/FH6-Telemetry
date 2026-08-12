#ifndef PARSE_ARGS_HPP
#define PARSE_ARGS_HPP

#include <variant>
#include <vector>

#include "../car_info.hpp"
#include "../engine_rpm.hpp"
#include "../gforce.hpp"
#include "../map.hpp"
#include "../race_info.hpp"
#include "../wheel_info.hpp"

using telemetry_variant_t = std::variant<car_info_t, engine_rpm_t, gforce_t, map_t, race_info_t, wheel_info_t>;

void print_help();
bool handle_telemetry_arg(std::string arg, std::vector<telemetry_variant_t>& telemetries);
int parse_args(int argc, const char* argv[], std::vector<telemetry_variant_t>& telemetries);

#endif
