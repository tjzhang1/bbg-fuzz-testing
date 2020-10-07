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

static uint8_t status;
static uint8_t i2c_fd;
static uint8_t buf[7];

/*Updates the status variable, returns a 1 if sensor is busy or 0 if sensor is idle.*/
int check_status(void) {
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
int init_sensor(int fd, int addr) {
   i2c_fd = fd;

   //setup the temperature and humidity sensor as a slave
   if(ioctl(i2c_fd, I2C_SLAVE, addr) < 0)
   {
      perror("Failed to acquire bus access and/or talk to slave\n");
      return(1);
   }
   printf("Connection established to sensor!\n");

   //initialize sensor with calibration settings
   uint8_t init_req[3] = {0xbe, 0x08, 0x00};
   if(write(i2c_fd, init_req, 3) != 3)
   {
      perror("Write operation failed\n");
      return(1);     
   }

   //wait until initialization finishes
   poll_until_done();
   printf("Sensor initialized!\n");
   
   return 0;
}

/*Prints if the sensor is calibrated or not*/
int calibrate_sensor(void) {
   if(check_status() < 0)
      return 1;

   //check calibration of sensor
   printf("Status: 0x%x\n", status);
   if(status & 0b00001000)
      printf("Sensor is calibrated\n");
   else
      printf("Sensor not calibrated\n");

   return 0;
}

/*
 *Sends the AHT20 a request to get measurements. The function polls until the
 *measurement is complete. It updates the global buf variable with the data.
*/
int measure(void) {
   //send measurement request
   uint8_t measure_req[3] = {0xac, 0x33, 0x00};
   if(write(i2c_fd, measure_req, 3) != 3)
   {
      perror("Write operation failed\n");
      return(1);     
   }

   //wait until sensor is no longer busy  
   poll_until_done();
   printf("Measurement complete!\n");

   //read measurements
   if(read(i2c_fd, buf, 7) != 7)
   {
      perror("Read operation failed\n");
      return(1);
   }
   printf("Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]); 

   parse_data(buf);

   return 0;
}

/*Prints data from the sensor as readable values*/
int parse_data(uint8_t data[7]) {
   int humidity = 0;
   int temp = 0;
   
   humidity |= (data[1] << 12);
   humidity |= (data[2] << 4);
   humidity |= (data[3] >> 4);
   
   temp |= ((data[3] & 0x0f) << 16);
   temp |= (data[4] << 8);
   temp |= data[5];

   //printf("Hum_int: 0x%05x, temp_int: 0x%05x\n", humidity, temp);
   printf("Humidity: %.3f%%, Temperature %.3f C\n", HUM_CONVERSION(humidity), TEMP_CONVERSION(temp));
  
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
   printf("Reset complete!\n");  

   return 0;
}

/*Continuously reads from the sensor until the sensor is idle*/
int poll_until_done(void) {
   int busy;

   while( (busy = check_status()) ) {
      if(busy < 0)
         return 1;
      //printf("Waiting...\n");
   };
   return 0;
}
