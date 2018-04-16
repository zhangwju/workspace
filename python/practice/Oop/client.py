#!/usr/bin/env python3.5
#coding=utf-8

import socket

class client:

	def __init__(self, host, port):
		self.host = host
		self.port = port
		self.fd = -1

	def init_socket(self, host, port):
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.connect((host, port))
		self.fd = s

	def start_client(self, host, port):
		self.init_socket(host, port)

if __name__ == '__main__':
	c = client("127.0.0.1", 9999)
	c.start_client('127.0.0.1', 9999)
	data = 'hello server'
	c.fd.send(data)
	data = c.fd.recv(1024)
	print("recv data: ", data)
