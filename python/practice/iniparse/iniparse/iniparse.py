#!/usr/bin/env python3.5
#coding=utf-8
#Author: zhangwju
#Date: 2017-12-20
#Email: zhangwju@gmail.com

import os

class Config:

	'''
	iniparse for python
	'''
	data = [] 
	cmap = {}
	def __init__(self, path):
		self.path=path
		if not os.access(path, os.F_OK):
			raise Exception("File Not Found", path)
			
		if not os.access(path, os.R_OK):
			raise Exception("File don't have read permission", path)
			
		with open(path, "r") as f:
			for line in f.readlines():
				line = line.strip()
				if not len(line) or line.startswith('#'):
					continue
				self.data.append(line)

	def parse(self, seq="="):
		for line in self.data:
			if seq in line:
				key, value = line.split(seq)
				self.cmap[key.strip()] = value.strip()

	def getstring(self, key):
		if key in self.cmap: 
			return self.cmap[key]
		else:
			print("%s no option" % key)

	def getint(self, key):
		if key in self.cmap:
			return int(self.cmap[key])
		else:
			print("%s no option" % key)
			return -1

	def getfloat(self, key):
		if key in self.cmap:
			return float(self.cmap[key])
		else:
			print("%s no option" % key)
			return -1.0
		
	def dumps(self):
		print("%s: %s" %  (self.path, self.data))
