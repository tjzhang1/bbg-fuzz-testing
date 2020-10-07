# Fuzzing with Boofuzz
| Tyler Zhang | tjzhang@iu.edu | GitHub: [:cloud:](https://github.iu.edu/tjzhang/beaglebone_green) |

## Introduction
### What is Fuzzing?
Fuzzing is a form of black-box bug testing that involves feeding random or unexpected input to some program. The program is monitored for observable failures such as crashes and hangs. Identifying input permutations that cause unexpected behavior can expose bugs and vulnerabilities in the target program.

### Embedded Systems
The rise of embedded systems in edge devices has raised security concerns. With simpler architectures and fewer resources available, embedded devices are not as secure as traditional desktop programs. Vulnerabilities in an edge device may give a malicious user an entry point for a core server or network. Thus, fuzz-testing embedded firmware can be a security test to catch vulnerabilities before they cause problems.

However, the process for fuzz-testing embedded systems is greatly underdeveloped. Many devices are highly specialized for performance and are stripped of features to maximize cost. As a result, it is particularly difficult to develop a one-size-fits-all approach to fuzzing embedded systems.

### Purpose
My work this summer was to use desktop tools to fuzz an embedded program running on the Beaglebone Green. In the process, I documented my workflow and the challenges that I encountered to evaluate the effectiveness of the fuzzing tool at catching vulnerabilities.

## Materials and Methods
### Platform
As mentioned in the previous section, I worked with the Beaglebone Green, a board that uses a 32-bit ARM processor and runs Linux. The Beaglebone comes preloaded with `gcc` and a Python compiler for software development. I connected the [AHT20](https://www.seeedstudio.com/Grove-AHT20-I2C-Industrial-grade-temperature-and-humidity-sensor-p-4497.html) I2C humidity and temperature sensor to collect data. I worked off of a 32 GB microSD card which gave me enough memory to download any Python packages.

For the fuzzing, I used the Python package called [boofuzz](https://github.com/jtpereyda/boofuzz). I chose boofuzz because it is simple and relatively easy to set up compared to the alternatives. I ran my fuzzing script from a separate Linux machine, which connected to my server program (described in the next section) on the Beaglebone through the TCP sockets interface.

### Buggy Server
[link to code](https://github.iu.edu/tjzhang/beaglebone_green/tree/master/httpserver)  
For the target program, I developed a simple HTTP server in C. One thread continuously collects data from the AHT20 sensor, and another thread handles HTTP requests from clients. Clients can interface with the sensor by setting calibration values, requesting readings, and checking its status. Clients can also send commands to close the server or reset the sensor. For a full list of features, see the [README](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/httpserver/README.md).

I planted several types of bugs in my server code. They include:
 - `printf` vulnerability
    - occurs by printing client input directly with `printf(input)` instead of `printf("%s", input)`
    - if `input` contains any valid `printf` format specifiers (such as `%s` or `%d`), the program will attempt to access memory from stack
 - heap-based buffer overflow
   - occurs when client requests too many sensor readings from server, overflowing a `malloc`'d array
 - stack-based buffer overflow
   - occurs when client requests too many sensor readings from server, overflowing a local array
 - double free
   - occurs when client requests the following URI: `/\/\/`
 - null pointer dereference
   - occurs when client requests the following URI: `/NULL`

These are the bugs that I hoped to catch with my fuzzing script.

### Debugging
Early on in the fuzz-testing process, I caught an unexpected bug that I could not identify. To catch it, I ran `gdbserver` on the Beaglebone and connected to it remotely from my Linux system running `gdb-multiarch`, which let me examine the stack. The bug occurs because when the server reads input from the client, it does not terminate the buffer with the `\0` character. Consequently, string functions such as `strtok` and `sscanf` continue to read until they find the next `\0` to indicate the end of the buffer, which could be out bounds for the array. The resulting behavior is a stack corruption.

### boofuzz Fuzzing Script
[link to code](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/boofuzz/httpFuzz.py)  
This is the Python script I used to fuzz my server. The directory also contains a file called [requirements.txt](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/boofuzz/requirements.txt), which can be used to install all the required Python packages with `pip`. This script generates test cases, sends test cases to my server, and logs the data into a neat `.db` file.

In the script, I defined five request formats that correspond with various server functions. In each request format, I specify the data types and their default values; then boofuzz generates test cases and permutations for me. A list of these request formats are summarized below.
 1. `GET / HTTP/1.0` as random bytes
 2. `GET / HTTP/1.0` as random string
 3. `GET /sensor?6 HTTP/1.0` with random string replacing the `6`
 4. `GET /\/\/ HTTP/1.0` with random string replacing the `/\/\/`
 5. `GET /NULL HTTP/1.0` with random string replacing the `/NULL`

### Tests
I ran four full fuzz tests, each time specifying different combinations of request formats and testing for different bugs in the server binary. Each test started with request formats 1 and 2 (random bytes and random string). In each test, the server also had the missing `\0` bug mentioned in the [Debugging section](#debugging).
 - Test 1: included request format 3 (tests the `/sensor?X` command)
   - Server is vulnerable to stack-based buffer overflow
 - Test 2: included request format 3
   - Server is vulnerable to heap-based buffer overflow
 - Test 3: included request format 4 (tests the `/\/\/` command)
   - Server is vulnerable to double free and `printf` bug
 - Test 4: included request format 5 (tests the `/NULL` command)
   - Server is vulnerable to null pointer dereference and `printf` bug

My testing was automated with the help of the built-in monitoring tool and callback functions, as described in the next section.

### Monitoring
I used the [boofuzz process monitor](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/httpserver/process_monitor_unix.py) to restart my server when it crashes or hangs. The script runs on the Beaglebone alongside the server. Although the process monitor can detect crashes locally, I used callback functions in my boofuzz fuzzing script to detect crashes remotely instead. This way, my fuzzing script can still identify crashes even if the process monitor were replaced by some other restarting mechanism.

The two callback functions both occur immediately after a fuzz test case is sent to my server. The first callback checks to make sure the server actually sent a response. The second callback pings the server by sending the request `GET /sensor?status HTTP/1.0` and checks to make sure the server response starts with `HTTP/1.0 200 OK`. If either of these callback checks fail, an error is logged and the fuzzing script sends a signal to the process monitor to restart the program.

## Results
### Test 1:
The missing `\0` bug and the stack-based buffer overflow bugs are caught. The following table shows how many test cases caused each type of bug:  
![graph1.1](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/fuzz_results/Run%202/graph1.png?raw=True)  
The following table shows which bugs are caught by request formats 1, 2, and 3 (see the [boofuzz fuzzing script](#boofuzz-fuzzing-script) section for a reminder of what these are).  
![graph1.2](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/fuzz_results/Run%202/graph2.png?raw=True)

### Test 2:
The missing `\0` bug and the heap-based buffer overflow bugs are caught. The following table shows how many test cases caused each type of bug:  
![graph2.1](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/fuzz_results/Run%203/graph1.png?raw=True)  
The following table shows which bugs are caught by request formats 1, 2, and 3 (see the [boofuzz fuzzing script](#boofuzz-fuzzing-script) section for a reminder of what these are).  
![graph2.2](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/fuzz_results/Run%203/graph2.png?raw=True)

### Test 3:
The missing `\0` bug, `printf` vulnerability, and double free bugs are caught. Further inspection on the failed test cases shows that every request for the `/\/\/` command results in a crash due to a double free. The following table shows how many test cases caused each type of bug:  
![graph3.1](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/fuzz_results/Run%204/graph1.png?raw=True)  
The following table shows which bugs are caught by request formats 1, 2, and 4 (see the [boofuzz fuzzing script](#boofuzz-fuzzing-script) section for a reminder of what these are).  
![graph3.2](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/fuzz_results/Run%204/graph2.png?raw=True)

### Test 4:
The missing `\0` bug, `printf` vulnerability, and null pointer dereference bugs are caught. Further inspection on the failed test cases shows that every request for the `/NULL` command results in a crash due to a null pointer dereference. The following table shows how many test cases caused each type of bug:  
![graph4.1](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/fuzz_results/Run%205/graph1.png?raw=True)  
The following table shows which bugs are caught by request formats 1, 2, and 5 (see the [boofuzz fuzzing script](#boofuzz-fuzzing-script) section for a reminder of what these are).  
![graph4.2](https://github.iu.edu/tjzhang/beaglebone_green/blob/master/python/fuzz_results/Run%205/graph2.png?raw=True)
## Discussion
All of the bugs planted in the server binary are caught by the tests. The double free and null pointer dereference bugs are caught every time their respective commands are sent to the server. Inspection of these test cases indicate that every time these bugs exist in the flow of execution, a crash occurs. Thus many of test cases from request formats 4 and 5 result in a crash.

The missing `\0` bugs and `printf` vulnerability also comprise the majority of the erroneous cases. The missing `\0` is triggered when the server is sent an input with length greater than its internal buffer size, 512. The `printf` bug is triggered when input contains valid format specifiers. These conditions happen relatively frequently when boofuzz generates permutations.

Finally, the stack-based and heap-based buffer overflow bugs occur when the client request a large number of sensor readings. This happens frequently in request format 3, in which the number of requested readings `6` is replaced with `1111111...` of various lengths.

### Consistency
The results from the previous section show the number of times each bug is caught by boofuzz; the number of erroneous test cases are usually a small fraction of the total number of test cases sent to the server. However, the percentages do not correspond with how consistently boofuzz catches bugs. In fact, if a certain test configuration catches a bug, then in my experience running the fuzzing script again would catch the bug again, despite the random nature of the test case generation.

For example, tests 1-4 each contain the missing `\0` bug. Each test consistently catches this bug roughly 20 times via the random bytes and random string request formats. Similarly, tests 3 and 4 both contain the `printf` vulnerability. Both tests catch the bug with a nearly-identical distribution of request formats (see the second chart for tests 3 and 4, in the `printf` column).

Upon closer examination of the boofuzz code, this has to do with its test case generation for strings. The program starts from a library of base "troublesome" strings and then creates random permutations from it, such as concatenating multiple copies of the base into one large string. Thus, testing strings is not completely random and catching bugs is fairly consistent between different runs.

### Working with boofuzz
Many boofuzz features were straightforward to set up. One can generate a full fuzzing script in a matter of minutes; the [boofuzz GitHub](https://github.com/jtpereyda/boofuzz/tree/master) has many examples in their documentation for fuzzing TCP connections. Users do not have to worry about different mutations because boofuzz generates those under the hood. The results were logged automatically in a database file which was convenient for reviewing test cases.

To get stronger results though, there are a couple parameters that are worth adjusting. One is `Session.crash_threshold_element`, the maximum number of crashes allowed before an element is exhausted. Its default is three but this value can be increased to keep more error test cases. Similarly, there is `Session.crash_threshold_request`, the maximum number of crashes allowed before a request format is exhausted. Its default is twelve but this value should also be increased.

The monitoring process was troublesome. The boofuzz GitHub has an example for using the process monitor program, but following their code did not work for me initially. It turns out they had a bug that prevented the program from recognizing stop commands, but the developers have since fixed this issue. Besides this hiccup, however, setting up the process monitor was relatively straightforward on the Beaglebone.

Writing the callback functions to check server responses was tricky because boofuzz did not document the order of callback functions very well in relation to other events that happen during a test step. I had to experiment with all callback functions before I understood the order of operation. This is the basic layout of a test case:
 1. Test case begins.
 2. Connection to target is established.
 3. Functions in `Session.pre_send_callbacks=[]` occur.
 4. `function()` in `Session.connect(callback=function)` occurs.
 5. Data is sent to target.
 6. If `Session.receive_data_after_each_request` is set to `True`, read from the socket. Otherwise skip.
 7. Functions in `Session.post_test_case_callbacks=[]` occur.
 8. If a `post_test_case_callbacks` function logs an error, functions in `Session.restart_callbacks=[]` occur.
 9. Close target connection, unless `Session.reuse_target_connection` is `True`.
 10. At any point, if the target program starts or restarts via the process monitor, the functions in `Session.post_start_target_callbacks=[]` occur.

I determined that I needed the `post_test_case_callbacks` because they occur directly after data is sent. The callback functions for reading and pinging the server were easy to configure and worked as intended. If the callbacks detected an erroneous response, they could log an error, signaling the process monitor to restart the program. Once I got the callbacks and process monitor working, the fuzzing process was straightforward again.

### When to use boofuzz?
As a black-box fuzzing tool, boofuzz is most useful for catching bugs that cause observable failure states. Overall it is fairly easy to use. Programming the fuzzing script is quick, assuming the communication protocol is simple. Boofuzz is well-suited for catching memory bugs on the Beaglebone because programs run in user space and Linux generates faults for dangerous system operations.

One limitation is that boofuzz cannot pinpoint the cause for a failure other than correlating the crash to a test case input. I experienced this when I was trying to discover the missing `\0` bug. It was hard to understand the issue without using `gdb` to examine the stack corruption.

Another difficult scenario for boofuzz would be if an embedded program has a memory leak that does not show observable failure until later. The volume of input is more problematic than the content, making the bug difficult to recreate. Boofuzz would not be able to tell the user this.

Monitoring is also tricky without the help of the process monitor to restart the program. Some boards can only run one process at a time and cannot use the process_monitor_unix.py script. One would need specialized power cycling hardware to fully automate the fuzzing script again.

Finally, boofuzz only supports various socket or serial connections. But it does not support fuzzing a local binary that takes input from `stdin` for example. Thus boofuzz is not ideal for fuzzing traditional desktop programs. Also, if the request format or protocol is too complex, then defining the request format in the fuzzing script will be a hassle.

### AFL+QEMU user mode
An alternative fuzzer to boofuzz is American fuzzy lop. It is more sophisticated than boofuzz; it adds instrumentation at compile-time for target programs and uses genetic algorithms to mutate test cases. AFL only fuzzes local programs that take input from `stdin` or from a file.

Usually, AFL fuzzes programs that have the same architecture and instruction set as the host computer. But AFL provides a fuzz-testing mode called QEMU user mode, which utilizes the `QEMU` emulator to fuzz binaries without the extra compile-time instrumentation. According to AFL's documentation, this mode allows for the fuzzing of binaries built for other architectures, such as ARM.

In my experience however, this is tricky to set up. Running simple ARM programs on my x86 computer with just `QEMU` requires linking ARM system libraries. However, there is no standard option to handle this with AFL+QEMU. So while it may be possible to "cross-fuzz" binaries, it will take some work to make this possible.
