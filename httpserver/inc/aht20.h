#include <stdint.h>

#define HUM_CONVERSION(x) ((x)*100.0/1048576.0)
#define TEMP_CONVERSION(x) ((x)*200.0/1048576.0 - 50.0)
#define I2C_BUS "/dev/i2c-2"
#define SENSOR_ADDR 0x38


/*Updates the status variable, returns a 1 if sensor is busy or 0 if sensor is idle.*/
int check_status();

/*
 *Initializes an I2C device at address addr on the I2C bus at file descriptor 
 *fd. The device is loaded as slave. Additionally, the AHT20 is given a command
 *to recalibrate its settings.
*/
int init_sensor(int fd);

/*Checks status to see if calibration data is set. If not, set the initialize the sensor*/
int calibrate_sensor(int fd);

/*
 *Sends the AHT20 a request to get measurements. The function polls until the
 *measurement is complete. The address h and t are updated with humidity and
 *temperature readings respectively.
*/
int measure(double *h, double *t);

/*Continuously reads from the sensor until the sensor is idle*/
int poll_until_done();

/*Sends the sensor a request to reset*/
int soft_reset();

/*
 *Interprets data from the sensor as readable values, storing humidity and
 *temperature at h and t respectively.
 */
int parse_data(uint8_t data[7], double *h, double *t);

/*Sends the sensor a request to initialize with calibration settings*/
int write_init_req() ;
