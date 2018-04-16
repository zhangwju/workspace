#!/usr/bin/env python3.5
#coding=utf-8

import hashlib

db = {}

def get_md5(passwd):
	md = hashlib.md5()
	md.update(passwd.encode("utf8"))
	return  md.hexdigest()

def register():
	print('sign up')
	username = input('Please Input Username:')
	passwd = input('Please Input Password:')
	db[username] = get_md5(username + passwd + 'test')
	print('sign up success')
	
def login():
	print('sign in')

	name = input('Please Input Username:')
	passwd = input('Please Input Password:')
	
	try:
		if db[name] == get_md5(name + passwd + 'test'):
			print('sign in success')
		else:
			print('password error')
	except KeyError:
		print('%s does not exist' % name)
			
	
if __name__ == '__main__':

	while True:
		print('Please Input the following options:')
		print('1: sign in\n2: sign up\n3: quit\n')
		a = input('Please Input your choice: ')
		if int(a) == 1:
			login()
		elif int(a) == 2:
			register()
		elif int(a) == 3:
			break
		else:
			print('Input invalid, Please continue...')

