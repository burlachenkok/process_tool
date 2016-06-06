#!/usr/bin/env python2
import subprocess, sys
subprocess.call(["process_tool.exe", "-timeExecLimit", "3", "launch", "calc.exe"])
subprocess.call(["process_tool.exe", "-timeExecLimit", "5", "launch", "calc.exe"])
