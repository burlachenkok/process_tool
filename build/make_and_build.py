#!/usr/bin/env python
import subprocess, sys, os
os.environ["PATH"] += os.pathsep + '''C:/Program Files (x86)/CMake/bin''';
subprocess.call('cmake ./../ -G "Visual Studio 11 2012"')
subprocess.call('cmake --build . --clean-first --config Release')
subprocess.call('cmake --build . --clean-first --config Debug')
