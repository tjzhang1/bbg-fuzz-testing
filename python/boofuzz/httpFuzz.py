#!/usr/bin/env python
# coding: utf-8

from boofuzz import *



def ping_server(target, fuzz_data_logger, session, *args, **kwargs):
    target.open()
    #send target a check command
    target.send(b"GET /sensor?status HTTP/1.0")
    try:
        response = target.recv(512)
    except:
        fuzz_data_logger.log_fail("Unable to connect to target. Closing...")
        return

    #if empty response
    if not response:
        fuzz_data_logger.log_fail("Empty response, target may be hung. Closing...")
        return

    #remove everything after null terminator, and convert to string
    response = response[:response.index(0)].decode('utf-8')
    #print(">> Response:", response)
    fuzz_data_logger.log_info("response check...\n" + response)
    
    if "HTTP/1.0 200 OK\r\nServer: BeagleBone Green\r\n" not in response:
        fuzz_data_logger.log_fail("Bad response from server. Closing...")
        return

    fuzz_data_logger.log_info("Response looks good.")
    return

def check_response(target, fuzz_data_logger, session, *args, **kwargs):
    fuzz_data_logger.log_info("Checking test case response...")
    try:
        response = target.recv(512)
    except:
        fuzz_data_logger.log_fail("Unable to connect to target. Closing...")
        target.close()
        return

    #if empty response
    if not response:
        fuzz_data_logger.log_fail("Empty response, target may be hung. Closing...")
        target.close()
        return

    #remove everything after null terminator, and convert to string
    response = response[:response.index(0)].decode('utf-8')
    fuzz_data_logger.log_info("response check...\n" + response)
    target.close()
    return

def main():
    STR_LEN = 512 #max size of s_string elements
    MIN_LEN = 2  #min size of s_random elements
    NUM_MUT = 100  #number of mutations for each s_random block

    #connect to process monitor
#    procmon = ProcessMonitor("192.168.1.133", 26002)
#    procmon.set_options(
#        start_commands=["make start"], 
#        stop_commands=["echo stopping", "TERMINATE_PID"],
#        proc_name="server.sh"
#    )
    #connect to server
    session = Session(
        target=Target(
            connection=TCPSocketConnection("192.168.1.133", 4000),
#            monitors=[procmon]
        ),
        post_test_case_callbacks=[check_response, ping_server],
        crash_threshold_element=20,
        crash_threshold_request=60
    )
    
    #define various formats to be sent and fuzzed
    # format: random binary
    s_initialize(name="randomRequest")
    with s_block("Request-Line"):
        s_random("GET", name='METHOD', min_length=MIN_LEN, max_length=STR_LEN, num_mutations=NUM_MUT)
        s_delim(" ", name='space-1')
        s_random("/", name='Request-URI', min_length=MIN_LEN, max_length=STR_LEN, num_mutations=NUM_MUT)
        s_delim(" ", name='space-2')
        s_random("HTTP/1.0", name='Version', min_length=MIN_LEN, max_length=STR_LEN, num_mutations=NUM_MUT)
        s_static("\r\n", name="Request-Line-CRLF")
    s_static("\r\n", "Request-CRLF")
    
    # format: random string
    s_initialize(name="randomRequestSTR")
    with s_block("Request-Line"):
        s_string("GET", name='METHOD', max_len=STR_LEN)
        s_delim(" ", name='space-1')
        s_string("/", name='Request-URI', max_len=STR_LEN)
        s_delim(" ", name='space-2')
        s_string("HTTP/1.0", name='Version', max_len=STR_LEN)
        s_static("\r\n", name="Request-Line-CRLF")
    s_static("\r\n", "Request-CRLF")
    
    # format: request for acquiring sensor readings
    s_initialize(name="sensorRequest")
    with s_block("Request-Line"):
        s_group("METHOD", ['GET'])
        s_delim(" ", name='space-1')
        s_string("/sensor", name='Request-URI', max_len=STR_LEN)
        s_static("?", name='arg_separator')
        s_string("6", name='arguments', max_len=STR_LEN)
        s_delim(" ", name='space-2')
        s_group("VERSION", ['HTTP/1.0', 'HTTP/1.1'])
        s_static("\r\n", name="Request-Line-CRLF")
    s_static("\r\n", "Request-CRLF")

    # format: request for double free command
    s_initialize(name="doubleFreeRequest")
    with s_block("Request-Line"):
        s_group("METHOD", ['GET'])
        s_delim(" ", name='space-1')
        s_string("/\\/\\/", name='Request-URI', max_len=STR_LEN)
        s_delim(" ", name='space-2')
        s_group("VERSION", ['HTTP/1.0', 'HTTP/1.1'])
        s_static("\r\n", name="Request-Line-CRLF")
    s_static("\r\n", "Request-CRLF")

    # format: request for null ptr dereference command
    s_initialize(name="nullRequest")
    with s_block("Request-Line"):
        s_group("METHOD", ['GET'])
        s_delim(" ", name='space-1')
        s_string("/NULL", name='Request-URI', max_len=STR_LEN)
        s_delim(" ", name='space-2')
        s_group("VERSION", ['HTTP/1.0', 'HTTP/1.1'])
        s_static("\r\n", name="Request-Line-CRLF")
    s_static("\r\n", "Request-CRLF")

    # connect to server and send fuzzed messages based on formats defined above
    session.connect(s_get("randomRequest"))
    session.connect(s_get("randomRequestSTR"))
    session.connect(s_get("nullRequest"))
    session.connect(s_get("sensorRequest"))
    session.connect(s_get("doubleFreeRequest"))
    session.fuzz()


if __name__ == "__main__":
    main()


# In[ ]:




