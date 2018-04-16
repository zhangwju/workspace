#!/usr/bin/env python3.5
#coding=utf-8

import os
import subprocess

for line in os.popen('ifconfig -a').readlines():
	print("", line)

os.system("ls -a")

counts = subprocess.getoutput("ifconfig  | grep enp0s8 | wc -l")
print("count: ", counts)
