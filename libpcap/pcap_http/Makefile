#
# Create by zhangwj
#

CFLAGS = -g
LDFLAGS = -lpcap

OBJS = test.o
 
TARGET = pacp
RM = rm -f 

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:clean

clean:
	$(RM) $(TARGET) $(OBJS)
