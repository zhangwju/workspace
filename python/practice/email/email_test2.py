#!/usr/bin/env python3.5
# -*- coding: utf-8 -*-
"""
自定义发送邮件的模块
输入参数：
    title: 邮件标题
    messages: 邮件内容
    to_addr: 收件人，如果是多个收件人，将地址放在列表中即可
示例: 发邮件给两个人
    send_mail('这是标题','这是内容',['收件人1@qq.com','收件人2@163.com'])
"""
#邮件的模块
from email import encoders 
from email.header import Header
from email.mime.text import MIMEText 
from email.utils import parseaddr, formataddr
import smtplib

def send_mail(title,messages,to_addr=['572089387@qq.com'] ):
	print("start....")
	from_addr= r'1134602675@qq.com'  #设置发件人邮箱地址
	password = r'wenqiang123'  #发件人邮箱密码
	#SMTP服务器
	smtp_server = 'smtp.qq.com'  #设置SMTP服务器
	msg = MIMEText(messages, 'html', 'utf-8')
	#设置邮件主题（要先实例化msg后才能设置主题）
	msg['From'] = from_addr
	msg['To'] = ','.join(to_addr)  #据说这是一个bug，只有这样才能群发邮件
	#msg['To'] = to_addr  #据说这是一个bug，只有这样才能群发邮件
	msg['Subject'] = title 
	print("connect %s %d" % (smtp_server, 465))
	#连接服务器发送邮件
	server = smtplib.SMTP('%s:%d' % (smtp_server, 465))
	#server.starttls()  #开启加密传输
	server.set_debuglevel(1)
	server.login(from_addr, password)
	print("send")
	server.sendmail(from_addr,to_addr, msg.as_string())
	print("send over")
	server.quit() 

if __name__ == '__main__':
	send_mail('python', 'hello python')
