#!/usr/bin/env python3.5
#coding=utf-8

import os
import re

absPath = os.path.abspath(os.path.dirname(__file__))
print(absPath)
m=re.match(r'^(\d{3})-(\d{3,8})$', '010-12345')
#print(m.group(0))
#print(m.group(1))
#print(m.group(2))

def is_valid_email(addr):
	print(addr)
#	m = re.match(r'^([0-9a-zA-Z\_]+?)\@([0-9a-zA-Z]+?)\.([a-z]{2,3})$', addr)
	if re_email.match(addr):
		print("email: %s" % re_email.match(addr).group())
		print("ok")
	else:
		print("no ok")

re_email = re.compile(r'^([0-9a-zA-Z\_]+?)\@([0-9a-zA-Z]+?)\.([a-z]{2,3})$')
if __name__ == '__main__':
	is_valid_email('zhangwju@gmail.com')
	is_valid_email('zhangwju_1.@gmail.c')
	is_valid_email('zhangwju__@gmail.cn')
	is_valid_email('zhangwju___@gmail.commm')
