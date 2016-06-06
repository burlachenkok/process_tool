#!/usr/bin/env python2
import subprocess
import msvcrt, os

# https://docs.python.org/2/library/msvcrt.html
fname = "stdin.txt"
myFile = open(fname, "rb")

# get one line (seems that get one line in Python runtime, really under the hood get not only one line via ReadFile)
print "1st line>> ", myFile.readline()

# set file pos to start
myFile.seek(0, 0)

myFileHandle = msvcrt.get_osfhandle(myFile.fileno())
print "STDIN OS FILEHANDLE DECIMAL: %i HEX: 0x%X FNAME: '%s'" % (myFileHandle, myFileHandle, fname)
print "Press any key to start..."
raw_input()
subprocess.call(["process_tool.exe", "-wait", "-stdin_native", str(myFileHandle), "launch", "cat.exe"])
myFile.close()
