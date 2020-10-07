# A simple HTTP server
## Instructions:
1. To build the executables, run 'make'
2. To start the server at port 4000, run 'make start'
3. To connect the client, run 'make test'
4. (Optional) You can also use a browser and navigate to the IP of the Beaglebone. Syntax:
 * *IP:port/* or *IP:port/sensor*: returns humidity and temperature readings
 * *IP:port/sensor?status*: returns whether sensor is busy or idle
 * *IP:port/sensor?reset*: command to tell sensor to reset

## Bugs:
 - No text allowed with length greater than BUF_LEN (hardcoded buffer size for reads/writes)
 - Unrecognized characters in string (untested)
 - Too many clients (untested)
