#!/usr/bin/env python
import os, shutil, glob

def removeFolder(folder):
    if os.path.isdir(folder):
        shutil.rmtree(folder)
        print "Folder ", folder, " cleaning completed..."
    else:
        print "Folder ", folder, " does not exist..."

def removeFiles(rule):
    for f in glob.glob(rule):
        print "Remove ", f
        os.remove(f)

removeFolder("./Debug")
removeFolder("./Release")
removeFolder("./Win32")
removeFolder("./CmakeFiles")
removeFolder("./process_tool.dir")
removeFiles("*.vcproj*")
removeFiles("*.vcxproj*")
removeFiles("*.cmake")
removeFiles("*.txt")
removeFiles("*.sln")
