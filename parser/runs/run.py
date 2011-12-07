#!/usr/bin/python

import subprocess
import os, sys
import glob
import time

files = glob.glob('../bin/*')
command = "./run.sh"
processes = list()
max_processes = int(sys.argv[1])

for name in files:
    #print name
    processes.append(subprocess.Popen([command, os.path.basename(name)]))
    if len(processes) >= max_processes:
        os.wait()
        for p in processes:
            if(p.poll() is not None):
                processes.remove(p);
                break;

#print len(processes)

while True:
    try:
        os.wait()
    except OSError:
        break

#print 'done'






