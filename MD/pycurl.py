#!/usr/bin/env python2
#encoding=utf8
import pycurl
import StringIO
# 安装pycurl到http://pycurl.sourceforge.net/这里去找.
# 在windows安装的话http://pycurl.sourceforge.net/download/ , 看你使用的版本决定下载那个，我在 windows使用的是python2.4, 所以下载 pycurl-ssl-7.15.5.1.win32-py2.4.exe 。
def test(debug_type, debug_msg):
print "debug(%d): %s" % (debug_type, debug_msg)
def postFile(url,post_file):
#print pycurl.version_info()
#这个函数创建一个同 libcurl中的CURL处理器相对应的Curl对象.Curl对象自动的设置CURLOPT_VERBOSE为0, CURLOPT_NOPROGRESS为1,提供一个默认的CURLOPT_USERAGENT和设置CURLOPT_ERRORBUFFER指向一个私有的错误缓冲区.
c = pycurl.Curl() #创建一个同libcurl中的CURL处理器相对应的Curl对象
b = StringIO.StringIO()
#c.setopt(c.POST, 1)
c.setopt(pycurl.URL, url) #设置要访问的网址 url = "http://www.cnn.com"
#写的回调
c.setopt(pycurl.WRITEFUNCTION, b.write)
c.setopt(pycurl.FOLLOWLOCATION, 1) #参数有1、2
#最大重定向次数,可以预防重定向陷阱
c.setopt(pycurl.MAXREDIRS, 5)
#连接超时设置
c.setopt(pycurl.CONNECTTIMEOUT, 60) #链接超时
# c.setopt(pycurl.TIMEOUT, 300) #下载超时
# c.setopt(pycurl.HEADER, True)
# c.setopt(c.HTTPHEADER, ["Content-Type: application/x-www-form-urlencoded","X-Requested-With:XMLHttpRequest","Cookie:"+set_cookie[0]])
#模拟浏览器
c.setopt(pycurl.USERAGENT, "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322)")
# c.setopt(pycurl.AUTOREFERER,1)
# c.setopt(c.REFERER, url)
# cookie设置
# Option -b/--cookie <name=string/file> Cookie string or file to read cookies from
# Note: must be a string, not a file object.
# c.setopt(pycurl.COOKIEFILE, "cookie_file_name")
# Option -c/--cookie-jar Write cookies to this file after operation
# Note: must be a string, not a file object.
# c.setopt(pycurl.COOKIEJAR, "cookie_file_name")
# Option -d/--data HTTP POST data
post_data_dic = {"name":"value"}
c.setopt(c.POSTFIELDS, urllib.urlencode(post_data_dic))
#设置代理
# c.setopt(pycurl.PROXY, ‘http://11.11.11.11:8080′)
# c.setopt(pycurl.PROXYUSERPWD, ‘aaa:aaa’)
#不明确作用
# c.setopt(pycurl.HTTPPROXYTUNNEL,1) #隧道代理
# c.setopt(pycurl.NOSIGNAL, 1)
#设置post请求， 上传文件的字段名 上传的文件
#post_file = "/home/ubuntu/avatar.jpg"
c.setopt(c.HTTPPOST, [("textname", (c.FORM_FILE, post_file))])
# 调试回调.调试信息类型是一个调试信 息的整数标示类型.在这个回调被调用时VERBOSE选项必须可用
# c.setopt(c.VERBOSE, 1) #verbose 详细
# c.setopt(c.DEBUGFUNCTION, test)
# f = open("body", "wb")
# c.setopt(c.WRITEDATA, f)
# h = open("header", "wb")
# c.setopt(c.WRITEHEADER, h)
# print "Header is in file 'header', body is in file 'body'"
# f.close()
# h.close()
# c.setopt(c.NOPROGRESS, 0)
# c.setopt(c.PROGRESSFUNCTION, progress)
# c.setopt(c.OPT_FILETIME, 1)
#访问,阻塞到访问结束
c.perform() #执行上述访问网址的操作
print "HTTP-code:", c.getinfo(c.HTTP_CODE) #打印出 200(HTTP状态码)
print "Total-time:", c.getinfo(c.TOTAL_TIME)
print "Download speed: %.2f bytes/second" % c.getinfo(c.SPEED_DOWNLOAD)
print "Document size: %d bytes" % c.getinfo(c.SIZE_DOWNLOAD)
print "Effective URL:", c.getinfo(c.EFFECTIVE_URL)
print "Content-type:", c.getinfo(c.CONTENT_TYPE)
print "Namelookup-time:", c.getinfo(c.NAMELOOKUP_TIME)
print "Redirect-time:", c.getinfo(c.REDIRECT_TIME)
print "Redirect-count:", c.getinfo(c.REDIRECT_COUNT)
# epoch = c.getinfo(c.INFO_FILETIME)
#print "Filetime: %d (%s)" % (epoch, time.ctime(epoch))
html = b.getvalue()
print(html)
b.close()
c.close()