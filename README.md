# ProcessTool

To bild and run:

# Create empty folder for autogenerated projects:
# Run cmake to generate project files for Windows, e.g. 'cmake "PATH TO THIS FOLDER" -G "Visual Studio 12 2013 Win64"'
# Build and run from visual studio ide
# Example run scripts are situated in "scripts2run"

This tool allow:
# Store process and it's child in container(sandbox) to processes, implemented via Job kernel object. Is is impossible to process to leave the JOB.
# Setup timeout for execution. If process create other child processes then they also will be killed by timeout.
# Execute killall to kill all processes in Sandbox
# Force process temination if unhandled SEH was happend
# List process collection which was created in this job and kill them all
# Take statistics of execution

// Copyright (c) 2016, Konstantin Burlachenko (burlachenkok@gmail.com).  All rights reserved.
