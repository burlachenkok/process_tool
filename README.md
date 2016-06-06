# ProcessTool 

It's a small tool to launch a process in Windows OS family and then kill process and all it's descendant proccesses.

#Prerequisites
 You should have installed python 2.7, cmake 2.8 or higher, and some version of Visual Studio

#To build:
 Goto build folder. Make changes in 'make_and_build.py' with you version of Visual Studio. Start 'make_and_build.py'

#To clean build folder:
 Goto build folder. Call 'clean.py'

#This tool allow:
1. Store process and it's child in container(sandbox) to processes, implemented via Job kernel object. Is is impossible to process to leave the JOB.
2. Setup timeout for execution. If process create other child processes then they also will be killed by timeout.
3. Execute killall to kill all processes in Sandbox
4. Force process temination if unhandled SEH was happend
5. List process collection which was created in this job and kill them all
6. Take statistics of execution

// Copyright (c) 2016, Konstantin Burlachenko (burlachenkok@gmail.com).  All rights reserved.
