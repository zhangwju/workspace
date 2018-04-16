#!/usr/bin/env python3.5
#coding=utf-8

class pkt:
	b = [0xEF,0x02,0xAA,0xAA]

a=pkt.b[:]
print(a)

a="hello world"
print(a.count("he"))
len = a.center(16, "2")
print(a)
