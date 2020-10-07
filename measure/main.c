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

#define I2C_BUS "/dev/i2c-2"
#define SENSOR_ADDR 0x38

#include "aht20.h"

int main(void)
{
   int fd;

   //open bus
   if ((fd=open(I2C_BUS, O_RDWR)) < 0)
   {
      perror("Failed to open i2c bus");
      exit(1);
   }
   printf("Connected to i2c bus!\n");
   
   if( init_sensor(fd, SENSOR_ADDR) )
      exit(1);
  
   if( calibrate_sensor() )
      exit(1); 

   if( measure() )
      exit(1);     
   
   //close before exiting
   close(fd);
   exit(0);
}
