#!/usr/bin/python

import subprocess
import os, sys
import glob
import time

files = glob.glob('../bin/*')
command = "./run.sh"
processes = set()
max_processes = int(sys.argv[1])

for name in files:
    processes.add(subprocess.Popen([command, name]))
    if len(processes) >= max_processes:
        os.wait()
        processes.difference_update(
            p for p in processes if p.poll() is not None)

#print len(processes)

while True:
    try:
        os.wait()
    except OSError:
        break

#print 'done'






