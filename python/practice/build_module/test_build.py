#!/usr/bin/env python3.5
#encoding=utf-8

from datetime import datetime

if __name__ == '__main__':
	now = datetime.now()
	print(now)
	dt = datetime(2015, 4, 19, 12, 20) # 用指定日期时间创建datetime
	print(dt)	
	dt = datetime(2015, 4, 19) # 用指定日期时间创建datetime
	print(dt)	
