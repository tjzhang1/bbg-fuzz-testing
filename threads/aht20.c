#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#include "aht20.h"

static uint8_t i2c_fd;

/*Updates the status variable, returns a 1 if sensor is busy or 0 if sensor is idle.*/
int check_status(void) {
   uint8_t status;

   if(read(i2c_fd, &status, 1) != 1)
   {
      perror("Read operation failed\n");
      return(-1);
   }
   return ((status & 0b10000000) >> 7); //bit[7] is status of sensor
}

/*
 *Initializes an I2C device at address addr on the I2C bus at file descriptor 
 *fd. The device is loaded as slave. Additionally, the AHT20 is given a command
 *to recalibrate its settings.
*/
int init_sensor(int fd) {
   i2c_fd = fd;

   //setup the temperature and humidity sensor as a slave
   if(ioctl(i2c_fd, I2C_SLAVE, SENSOR_ADDR) < 0)
   {
      perror("Failed to acquire bus access and/or talk to slave\n");
      return(1);
   }

   //initialize sensor with calibration settings
   uint8_t init_req[3] = {0xbe, 0x08, 0x00};
   if(write(i2c_fd, init_req, 3) != 3)
   {
      perror("Write operation failed\n");
      return(1);     
   }

   //wait until initialization finishes
   poll_until_done();
   
   return 0;
}

/*Checks status to see if calibration data is set. If not, set the initialize the sensor*/
int calibrate_sensor(int fd) {
   uint8_t status;
   if( (status = check_status()) < 0)
      return 1;

   //check calibration of sensor
   if(status & 0b00001000)
      return 0;
   else
      return init_sensor(fd);
}

/*
 *Sends the AHT20 a request to get measurements. The function polls until the
 *measurement is complete. The address h and t are updated with humidity and
 *temperature readings respectively.
*/
int measure(double *humidity, double *temp) {
   uint8_t buf[7];
   //send measurement request
   uint8_t measure_req[3] = {0xac, 0x33, 0x00};
   if(write(i2c_fd, measure_req, 3) != 3)
   {
      perror("Write operation failed\n");
      return(1);     
   }

   //wait until sensor is no longer busy  
   poll_until_done();

   //read measurements, comes as 7 bytes
   if(read(i2c_fd, buf, 7) != 7)
   {
      perror("Read operation failed\n");
      return(1);
   }
//   printf("Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]); 

   parse_data(buf, humidity, temp);

   return 0;
}

/*
 *Interprets data from the sensor as readable values, storing humidity and
 *temperature at h and t respectively.
 */
int parse_data(uint8_t data[7], double *h, double *t) {
   int humidity = 0;
   int temp = 0;
   
   //Humidity stored as 2 bytes and 1 nibble
   //corresponds to second, third, and half of the fourth byte of data
   humidity |= (data[1] << 12);
   humidity |= (data[2] << 4);
   humidity |= (data[3] >> 4);
   
   //Temperature stored as 2 bytes and 1 nibble
   //corresponds to half of fourth, fifth, and sixth bytes of data
   temp |= ((data[3] & 0x0f) << 16);
   temp |= (data[4] << 8);
   temp |= data[5];

   *h = HUM_CONVERSION(humidity);
   *t = TEMP_CONVERSION(temp);
   return 0;
}

/*Sends the sensor a request to reset*/
int soft_reset(void) {
   uint8_t reset_req = 0xba;
   if(write(i2c_fd, &reset_req, 1) != 1)
   {
      perror("Write operation failed\n");
      return(1);     
   }

   //wait until sensor is no longer busy  
   poll_until_done();

   return 0;
}

/*Continuously reads from the sensor until the sensor is idle*/
int poll_until_done(void) {
   int busy;

   while( (busy = check_status()) ) {
      if(busy < 0)
         return 1;
   };
   return 0;
}
