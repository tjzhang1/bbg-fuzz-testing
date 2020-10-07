import subprocess
import os
import signal
import psutil
import time
import shlex

def _split_command_if_str(command):
    """Splits a shell command string into a list of arguments.
    If any individual item is not a string, item is returned unchanged.
    Designed for use with subprocess.Popen.
    Args:
        command (Union[basestring, :obj:`list` of :obj:`basestring`]): List of commands. Each command
        should be a string or a list of strings.
    Returns:
        (:obj:`list` of :obj:`list`: of :obj:`str`): List of lists of command arguments.
    """
    if isinstance(command, str):
        return shlex.split(command, posix=(os.name == "posix"))

    else:
        return command

#start the process
serverProcess = subprocess.Popen(['./server.sh', '4000'])
print("Waiting for process to start...")
time.sleep(2)
pid = serverProcess.pid
print("Done. PID:", pid)

while True:
    command = input(">> ")
    #exit command
    if "exit" in command:
        print("Killing process with pid:", pid) 
        try:
            os.kill(pid, signal.SIGKILL)
        except OSError as e:
            print(e)
        break
    #other commands
    else:
        command = _split_command_if_str(command)
        try:
            com = subprocess.Popen(command)
            os.waitpid(com.pid, 0)
        except:
            print("Task failed.")
            
