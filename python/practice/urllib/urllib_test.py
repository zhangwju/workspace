#!/usr/bin/env python3.5
#coding=utf-8

from urllib import request
with request.urlopen('https://www.baidu.com') as f:
	data=f.read()
	print('Status:', f.status, f.reason)
	for k, v in f.getheaders():
		print('%s: %s' % (k, v))
	print('Data:', data.decode('utf-8'))
