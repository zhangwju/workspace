#!/usr/bin/env python3.5
#coding=utf-8

import smtplib
from email.mime.text import MIMEText

msg_from='1134602675@qq.com'                                 #发送方邮箱
passwd='wenqiang123'                                   #填入发送方邮箱的授权码
msg_to='572089387@qq.com'                                  #收件人邮箱
                            
subject='python email test'									#主题     
content='hellon python'		#正文
msg = MIMEText(content)
msg['Subject'] = subject
msg['From'] = msg_from
msg['To'] = msg_to
try:
	s = smtplib.SMTP_SSL('smtp.qq.com',465)
	s.login(msg_from, passwd)
	s.sendmail(msg_from, msg_to, msg.as_string())
	print("发送成功")
except s.SMTPException:
	print("发送失败")
finally:
	s.quit()
