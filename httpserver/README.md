# A simple HTTP server
Note: All scripts are configured for the Beaglebone with a local IP address of 192.168.1.133. By default, `make start` starts the server at port 4000 and `make test` connects the client program to 192.168.1.133:4000.

## Instructions:
1. To build the executables, run `make` or `make all`
2. To start the server at port 4000, run `make start`
3. To connect the client, run `make test`. Type in a valid HTTP request, such as `GET / HTTP/1.0`.
4. (Optional) You can also use a browser and navigate to the IP of the Beaglebone.

## Request Line Syntax: *http://[IP]:[port]/[URI]*
 * `/` or `/sensor`: returns oldest humidity and temperature reading in queue
 * `/sensor?X`: returns *X* number of readings in queue, from oldest to newest
 * `/sensor?status`: returns whether sensor is busy or idle
 * `/sensor?reset`: command to tell sensor to reset
 * `/close`: command to turn off server
 * `/sensor?calibrate&humidity&f`: adjust humidity readings by a factor of *f* (i.e. humidity = (sensor_value)xf)
 * `/sensor?calibrate&temperature&f`: adjust temp readings by a factor of *f* (i.e. temp = (sensor_value)xf)
 * `/\/\/`: command to perform a double free
 * `/NULL`: command to perform a null pointer dereference

## Bugs:
 - printf vulnerability
 - heap-based buffer overflow
 - stack-based buffer overflow
 - no null terminator at end of input buffer
 - double free
 - null pointer dereference
 
 These bugs can be enabled/disabled on lines 22-24 of [server.c](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/httpserver/src/server.c).

## Monitoring:
 I've included the file [process_monitor_unix.py](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/httpserver/process_monitor_unix.py) which can also be found on the [boofuzz GitHub](https://github.com/jtpereyda/boofuzz) as well. If you decide to utilize process monitoring, make sure to install the required Python packages specified in the [requirements.txt](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/boofuzz/requirements.txt) file.
 
 Using the process monitor is pretty straightforward: on the Beaglebone, run the process_monitor_unix.py script, which is configured to run on 192.168.1.133:26002. Then, run the [httpFuzz.py](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/boofuzz/httpFuzz.py) script, which will connect to the process monitor and give it a signal to start the server. Make sure lines 62-67 and 72 are uncommented in httpFuzz.py.
