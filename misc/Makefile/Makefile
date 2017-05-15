#
#Create By zhangwj
#

CFLAGS := -g
#LDFLAGS +=

PIC = -fPIC
SHARED= -shared

OBJS= test.o

RM= /bin/rm -f
TARGET= libtest.so

%.o:%.c
	$(CC) $(CFLAGS) $(PIC) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(SHARED) -o $@ $^

.PHONY:clean

clean:
	$(RM) $(TARGET) $(OBJS)







