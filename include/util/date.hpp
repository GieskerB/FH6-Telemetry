#ifndef DATE_HPP
#define DATE_HPP

typedef struct {
    unsigned char day, month;
    unsigned short year;
} date;

date get_today();
unsigned int date_to_int(const date&);

#endif