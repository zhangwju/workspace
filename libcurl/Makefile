#
# Design of curlbat
#

CFLAGS = -g
LDFLAGS = -lcurl

OBJS = curlbat.o test.o
TARGET = curl-bat

#clean objs
RM = rm -f 
 
$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:clean

clean:
	$(RM) $(TARGET) $(OBJS)
