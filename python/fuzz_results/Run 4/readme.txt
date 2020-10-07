RUN 4:

process monitor used

Fuzz requests:
    session.connect(s_get("randomRequest"))
    session.connect(s_get("randomRequestSTR"))
    session.connect(s_get("doubleFreeRequest"))

Likely causes:
// accept() error probably still exists
// PIPE error probably still exists

Active bugs in server:
Stack-based buffer overflow, but hidden behind sensor?X command
Double free. Command is requested and fuzzed
Null ptr dereference, but hidden behind NULL command
printf vulnerability
No null terminator at the end of buffer upon reading from client

Parameters:
STR_LEN = 512 #max size of s_string elements
MIN_LEN = 2  #min size of s_random elements
NUM_MUT = 100  #number of mutations for each s_random block
crash_threshold_element=20
crash_threshold_request=60

