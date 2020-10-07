#include <stdint.h>

#define HUM_CONVERSION(x) ((x)*100.0/1048576.0)
#define TEMP_CONVERSION(x) ((x)*200.0/1048576.0 - 50.0)

int check_status();
int init_sensor(int fd, int addr);
int calibrate_sensor();
int measure();
int poll_until_done();
int soft_reset();
int parse_data(uint8_t data[7]);
