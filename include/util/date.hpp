#ifndef DATE_HPP
#define DATE_HPP

struct date {
    unsigned char day, month;
    unsigned short year;
};

date get_today();
unsigned int date_to_int(const date&);

#endif
