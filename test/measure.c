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

#define i2cBus "/dev/i2c-2"
#define i2cAddr 0x38


int main(void)
{
   int i2c_fd;
   
   //open the bus
   if ((i2c_fd=open(i2cBus, O_RDWR)) < 0)
   {
      perror("Failed to open i2c bus");
      exit(1);
   }
   printf("Connected to i2c bus!\n");
   
   //setup the temperature and humidity sensor as a slave
   if(ioctl(i2c_fd, I2C_SLAVE, i2cAddr) < 0)
   {
      perror("Failed to acquire bus access and/or talk to slave\n");
      exit(1);
   }
   printf("Connection established to sensor!\n");
   
   //read from sensor

   //initialize
   uint8_t init_req[3] = {0xbe, 0x08, 0x00};
   if(write(i2c_fd, init_req, 3) != 3)
   {
      perror("Write operation failed\n");
      exit(1);     
   }
   //wait until initialization finishes
   uint8_t status;
   do {
      if(read(i2c_fd, &status, 1) != 1)
      {
         perror("Read operation failed\n");
         exit(1);
      }
   } while (status & 0b10000000);
   printf("Sensor initialized\n");

   //check calibration of sensor
   printf("Status: 0x%x\n", status);
   if(status & 0b00001000)
      printf("Sensor is calibrated\n");
   else
      printf("Sensor not calibrated\n");

   //send measurement request
   uint8_t measure_req[3] = {0xac, 0x33, 0x00};
   if(write(i2c_fd, measure_req, 3) != 3)
   {
      perror("Write operation failed\n");
      exit(1);     
   }
   //wait until measurement completes
   do {
      if(read(i2c_fd, &status, 1) != 1)
      {
         perror("Read operation failed\n");
         exit(1);
      }
      printf("Waiting...\n");
   } while (status & 0b10000000);
   printf("Measurement complete!\n");
   //read measurements
   uint8_t buf[7];
   if(read(i2c_fd, buf, 7) != 7)
   {
      perror("Read operation failed\n");
      exit(1);
   }
   printf("Data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]); 
   //check if sensor is idle
   do {
      if(read(i2c_fd, &status, 1) != 1)
      {
         perror("Read operation failed\n");
         exit(1);
      }
   } while (status & 0b10000000);  
   printf("Sensor idle! Status: 0x%x\n", status);

   //close before exiting
   close(i2c_fd);
   exit(0);
}
