#!/usr/bin/env python3.5
#coding=utf-8

import subprocess

print('$ nslookup www.python.org')
r = subprocess.call(['ls', '-l'])
print('Exit code:', r)
