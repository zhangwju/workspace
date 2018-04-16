#!/usr/bin/env python3.5
#coding=utf-8

import os

print("Process (%s) start ...." % os.getpid())
pid = os.fork()
if pid == 0:
	print("I am child process (%s) and my parent is %s" % (os.getpid(), os.getppid()))
else:
	print("I am parent (%s) just create child process (%s)" % (os.getpid(), pid))
