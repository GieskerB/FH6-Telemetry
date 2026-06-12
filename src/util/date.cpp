#include <ctime>

#include "../../include/util/date.hpp"

date get_today() {
    const time_t t = time(nullptr);
    tm tm = *localtime(&t);
    return {
            static_cast<unsigned char>(tm.tm_mday), static_cast<unsigned char>(tm.tm_mon + 1),
            static_cast<unsigned short>(tm.tm_year + 1900)
    };
}

static constexpr bool is_leap_year(const unsigned short year) {
    if (year % 4 != 0) return false;
    if (year % 100 == 0) {
        if (year % 400 == 0) return true;
        return false;
    }
    return true;
}

static constexpr unsigned char days_in_month(const unsigned char month, const unsigned short year) {
    constexpr unsigned char days_per_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return month == 2 and is_leap_year(year) ? 29 : days_per_month[month - 1];
}

static constexpr unsigned short days_in_year(const unsigned short year) {
    return is_leap_year(year) ? 366 : 365;
}

unsigned int date_to_int(const date& date) {
    unsigned int result = 0;
    // Add all previous years
    for (unsigned short i = 0; i <= date.year;++i) {
        result += days_in_year(i);
    }
    for (unsigned char i = 1; i <= date.month;++i) {
        result += days_in_month(i, date.year);
    }
    result += date.day;
    return result;
}