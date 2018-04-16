#!/usr/bin/env python3.5
#coding=utf-8

import os
from iniparse.iniparse import *

if __name__ == '__main__':

	c = Config("./ini/test.ini")
	print("Class Info: ", c.__doc__.strip())
	c.dumps()
	c.parse("=")

	print("name: %s" % c.getstring("name"))
	print("age: %d" % c.getint("age"))
	print("sex: %s" % c.getstring("sex"))
	print("email: %s" % c.getstring("email"))
	print("bolg: %s" % c.getstring("blog"))
	print("frinds: %s" % c.getstring("friends"))
	print("money: %s" % c.getstring("money"))
