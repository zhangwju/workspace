#!/usr/bin/env python3.5
#coding=utf-8

from multiprocessing import Process
import os

def run_proc(name):
	print("Run child process %s (%s)..." % (name, os.getpid()))

if __name__ == '__main__':
	print("Parent process (%s) start ...." % os.getpid())
	p = Process(target=run_proc, args=('test',))
	print("Child process start..")
	p.start()
	p.join()
	print("Child process end..")
	
