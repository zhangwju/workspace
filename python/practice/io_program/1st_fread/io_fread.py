#!/usr/bin/env python3.5
# -*- coding: UTF-8 -*-

#1
f=open("./test_python", "r")
print(f.read())
f.close()

#2
try:
	f = open("./test_python", "r")
	print(f.read())
finally:
	print("finally")
	if f:
		f.close()

#3
with open("./test2", "r") as f:
	while True:
		s = f.readline()
		if s == '':
			break
		print(s.strip())

with open("./test2", "r") as f:
	l = f.readlines()
	for line in l:
		print(line.strip())	

import os
print("<------------------------------------->")

def search(name, path= os.path.abspath('')):
	d = (x for x in os.listdir(path))
	for x in d:
		x_fname = os.path.join(path, x)
		if os.path.isdir(x_fname):
			search(name, os.path.abspath(x_fname))
		elif os.path.isfile(x_fname) and name in os.path.split(x_fname)[1]:
			print('文件名：%s；相对路径：%s' % (os.path.split(x_fname)[1], os.path.dirname(x_fname).lstrip(os.path.abspath(''))))
		else:
			pass

search("test")


[x for x in os.listdir('.') if os.path.isfile(x) and os.path.splitext(x)[1]=='.py' and print('%s %s' % (x, os.path.splitext(x)[0]))]
