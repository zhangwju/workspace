#!/usr/bin/env python3.5
#coding=utf-8

from multiprocessing import Pool
import os, time, random

def run_proc(name):
	print("Task %s (%s) is running ..." % (name, os.getpid()))
	start = time.time()
	time.sleep(random.random() * 3)
	end = time.time()
	print('Task %s runs %0.2f seconds.' % (name, (end - start)))
	

if __name__ == '__main__':
	print("Parent process (%s) .." % os.getpid())

	p = Pool(4)
	for i in range(5):
		p.apply_async(run_proc, args=(i,))		
	print("Wait all subprocesses done...")
	p.close()
	p.join()
	print("All subprocesses done")
	
