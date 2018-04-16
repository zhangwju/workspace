#!/usr/bin/python

import socket

if __name__ == "__main__":
	s = socket.socket()
	host = socket.gethostname()
	port = 9999 

	s.connect((host, port))
	print(s.recv(1024))
	data = "hello world"
	data = data.encode()
	s.send(data)
	print(s.recv(1024).decode())
	s.close()
