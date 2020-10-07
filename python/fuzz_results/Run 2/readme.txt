RUN 2:

process monitor used

Fuzz requests:
    session.connect(s_get("randomRequest"))
    session.connect(s_get("randomRequestSTR"))
    session.connect(s_get("sensorRequest"))

Likely causes:
// accept() error probably still exists
// PIPE error probably still exists

Active bugs in server:
Stack-based buffer overflow
Double free and null ptr dereference, but these are hidden behind specific commands (not usually in flow of program)
No null terminator at the end of buffer upon reading from client

Parameters:
STR_LEN = 512 #max size of s_string elements
MIN_LEN = 2  #min size of s_random elements
NUM_MUT = 100  #number of mutations for each s_random block
crash_threshold_element=20
crash_threshold_request=60

