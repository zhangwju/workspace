#!/usr/bin/env python3.5
#coding=utf-8

import socket
import time, threading

def tcplink(sock, addr):
	print('Accept new connection from %s:%s...' % addr)
	sock.send(b'Welcome!')
	while True:
		data = sock.recv(1024)
		time.sleep(1)
		if not data or data.decode('utf-8') == 'exit':
			break
		print("cli: %s" % data.decode('utf-8'))
		sock.send(('你好, %s!' % data.decode('utf-8')).encode('utf-8'))
	sock.close()
	print('Connection from %s:%s closed.' % addr)

class server:

	def __init__(self, host, port):
		self.host = host
		self.port = port
		self.fd = -1

	def init_socket(self, host, port):
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.bind((host, port))
		s.listen(5)
		self.fd = s

	def start_server(self, host, port):
		self.init_socket(host, port)

if __name__ == '__main__':
	s = server("127.0.0.1", 9999)
	s.start_server('127.0.0.1', 9999)

	while True:
		# 接受一个新连接:
		sock, addr = s.fd.accept()
		# 创建新线程来处理TCP连接:
		t = threading.Thread(target=tcplink, args=(sock, addr))
		t.start()
