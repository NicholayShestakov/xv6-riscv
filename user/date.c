#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void
convert_to_beautiful_datetime_format(uint64 *nanoseconds)
{
  int days_per_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  uint64 seconds = *nanoseconds / 1000000000;
  uint64 minutes = (seconds / 60);
  uint64 hours = (minutes / 60);
  uint64 days = (hours / 24);
  uint64 years = 1970;

  while (1) {
    int has_366_days =
      (years % 4 == 0 && (years % 100 != 0 || years % 400 == 0));
    uint64 days_in_year = has_366_days ? 366 : 365;
    if (days < days_in_year)
      break;
    days -= days_in_year;
    years++;
  }

  if (years % 4 == 0 && (years % 100 != 0 || years % 400 == 0)) {
    days_per_month[1] = 29;
  }
  int month_number = 0;

  while (month_number < 12 && days >= (uint64)days_per_month[month_number]) {
    days -= days_per_month[month_number];
    month_number++;
  }
  uint64 day = days + 1;
  uint64 month = month_number + 1;

  if (day < 10) {
    printf("0");
  }
  printf("%lu.", day);

  if (month < 10) {
    printf("0");
  }
  printf("%lu.%lu", month, years);

  if ((hours % 24) < 10) {
    printf("0");
  }
  printf(" %lu:", hours % 24);

  if ((minutes % 60) < 10) {
    printf("0");
  }
  printf("%lu:", minutes % 60);

  if ((seconds % 60) < 10) {
    printf("0");
  }
  printf("%lu.%lu\n", seconds % 60, (*nanoseconds % 1000000000));
}

void
main()
{
  uint64 nanoseconds;
  int fd = open("rtc", 0);
  if (fd < 0) {
    printf("Problem with creating fd\n");
    exit(-1);
  }
  if (read(fd, &nanoseconds, 8) != 8) {
    printf("Problem with reading registers\n");
    exit(-1);
  }
  convert_to_beautiful_datetime_format(&nanoseconds);
  close(fd);
}